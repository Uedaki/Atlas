#pragma once

#include "atlas/Atlas.h"
#include "atlas/core/Geometry.h"
#include "atlas/core/Points.h"
#include "atlas/core/Reflection.h"
#include "atlas/core/Microfacet.h"
#include "atlas/core/RgbSpectrum.h"
#include "atlas/core/Sampling.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
	class Fresnel
	{
	public:
		virtual ~Fresnel() = default;
		virtual Spectrum evaluate(Float cosI) const = 0;
	};

	class FresnelConductor : public Fresnel
	{
	public:
		FresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k)
			: etaI(etaI), etaT(etaT), k(k)
		{}
		Spectrum evaluate(Float cosThetaI) const override
		{
			return (fresnelConductor(std::abs(cosThetaI), etaI, etaT, k));
		}

	private:
		Spectrum etaI;
		Spectrum etaT;
		Spectrum k;
	};

	class FresnelDielectric : public Fresnel
	{
	public:
		FresnelDielectric(Float etaI, Float etaT)
			: etaI(etaI), etaT(etaT)
		{}

		Spectrum evaluate(Float cosThetaI) const override
		{
			return (fresnelDielectric(cosThetaI, etaI, etaT));
		}

	private:
		Float etaI;
		Float etaT;
	};

	class FresnelNoOp : public Fresnel
	{
	public:
		Spectrum evaluate(Float) const override
		{
			return Spectrum(1.f);
		}
	};

	enum BxDFType
	{
		BSDF_REFLECTION = 1 << 0,
		BSDF_TRANSMISSION = 1 << 1,
		BSDF_DIFFUSE = 1 << 2,
		BSDF_GLOSSY = 1 << 3,
		BSDF_SPECULAR = 1 << 4,
		BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION | BSDF_TRANSMISSION
	};

	class BxDF
	{
	public:
		BxDF(BxDFType type)
			: type(type)
		{}
		virtual ~BxDF() = default;

		bool matchesFlags(BxDFType t) const
		{
			return (type & t) == type;
		}

		virtual Spectrum f(const Vec3f &wo, const Vec3f &wi) const = 0;
		virtual Spectrum sampleF(const Vec3f &wo, Vec3f &wi, const Point2f &sample, Float &pdf, BxDFType *sampledType = nullptr) const
		{
			wi = cosineSampleHemisphere(sample);
			if (wo.z < 0)
				wi.z *= -1;
			pdf = BxDF::pdf(wo, wi);
			return f(wo, wi);
		}
		virtual Spectrum rho(const Vec3f &wo, int nSamples, const Point2f *samples) const
		{
			Spectrum r(0.);
			for (int i = 0; i < nSamples; ++i)
			{
				Vec3f wi;
				Float pdf = 0;
				Spectrum f = sampleF(wo, wi, samples[i], pdf);
				if (pdf > 0)
					r += f * absCosTheta(wi) / pdf;
			}
			return r / (Float)nSamples;
		}
		virtual Spectrum rho(int nSamples, const Point2f *samples1, const Point2f *samples2) const
		{
			Spectrum r(0.f);
			for (int i = 0; i < nSamples; ++i)
			{
				Vec3f wo, wi;
				wo = uniformSampleHemisphere(samples1[i]);
				Float pdfo = uniformHemispherePdf(), pdfi = 0;
				Spectrum f = sampleF(wo, wi, samples2[i], pdfi);
				if (pdfi > 0)
					r += f * absCosTheta(wi) * absCosTheta(wo) / (pdfo * pdfi);
			}
			return r / (PI * nSamples);
		}

		virtual Float pdf(const Vec3f &wo, const Vec3f &wi) const
		{
			return sameHemisphere(wo, wi) ? absCosTheta(wi) * INV_PI : 0;
		}

		const BxDFType type;
	};

	class ScaledBxDF : public BxDF
	{
	public:
		ScaledBxDF(BxDF *bxdf, const Spectrum &scale)
			: BxDF(BxDFType(bxdf->type)), bxdf(bxdf), scale(scale)
		{}

		Spectrum f(const Vec3f &wo, const Vec3f &wi) const override
		{
			return (scale * bxdf->f(wo, wi));
		}

		Spectrum sampleF(const Vec3f &wo, Vec3f &wi, const Point2f &sample, Float &pdf, BxDFType *sampledType = nullptr) const override
		{
			return (scale * bxdf->sampleF(wo, wi, sample, pdf, sampledType));
		}

		Spectrum rho(const Vec3f &wo, int nSamples, const Point2f *samples) const override
		{
			return (scale * bxdf->rho(wo, nSamples, samples));
		}

		Spectrum rho(int nSamples, const Point2f *samples1, const Point2f *samples2) const override
		{
			return (scale * bxdf->rho(nSamples, samples1, samples2));
		}

	private:
		BxDF *bxdf;
		Spectrum scale;
	};

	class SpecularReflection : public BxDF
	{
	public:
		SpecularReflection(const Spectrum &r, const Fresnel &fresnel)
			: BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR))
			, r(r), fresnel(fresnel)
		{}

		Spectrum f(const Vec3f &wo, const Vec3f &wi) const override
		{
			return (Spectrum(0.f));
		}

		Spectrum sampleF(const Vec3f &wo, Vec3f &wi, const Point2f &sample, Float &pdf, BxDFType *sampledType) const override
		{
			wi = Vec3f(-wo.x, -wo.y, wo.z);
			pdf = 1.f;
			return (r);// fresnel.evaluate(cosTheta(wi)) *r / absCosTheta(wi));
		}

		Float pdf(const Vec3f &wo, const Vec3f &wi) const override
		{
			return (0.f);
		}

	private:
		const Spectrum r;
		const Fresnel &fresnel;
	};

	class SpecularTransmission : public BxDF
	{
	public:
		SpecularTransmission(const Spectrum &t, Float etaA, Float etaB, TransportMode mode)
			: BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR))
			, t(t), etaA(etaA), etaB(etaB), fresnel(etaA, etaB)
			, mode(mode)
		{}

		Spectrum f(const Vec3f &wo, const Vec3f &wi) const override
		{
			return (0.f);
		}

		Spectrum sampleF(const Vec3f &wo, Vec3f &wi, const Point2f &sample, Float &pdf, BxDFType *sampledType) const override
		{
			bool entering = cosTheta(wo) > 0;
			Float etaI = entering ? etaA : etaB;
			Float etaT = entering ? etaB : etaA;

			if (!refract(wo, faceForward(Normal(0, 0, 1), wo), etaI / etaT, wi))
				return (0);

			pdf = 1.f;
			return (Spectrum(1.f));
			Spectrum ft = t * (Spectrum(1.f) - fresnel.evaluate(cosTheta(wi)));
			if (mode == TransportMode::Radiance)
				ft *= (etaI * etaI) / (etaT * etaT);
			return (ft / absCosTheta(wi));
		}

		Float pdf(const Vec3f &wo, const Vec3f &wi) const override
		{
			return (0.f);
		}

	private:
		const Spectrum t;
		const Float etaA;
		const Float etaB;
		const FresnelDielectric fresnel;
		const TransportMode mode;
	};

	class FresnelSpecular : public BxDF
	{
	public:
		FresnelSpecular(const Spectrum &r, const Spectrum &t, Float etaA, Float etaB, TransportMode mode)
			: BxDF(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR))
			, r(r), t(t), etaA(etaA), etaB(etaB), fresnel(etaA, etaB), mode(mode)
		{}

		Spectrum f(const Vec3f &wo, const Vec3f &wi) const override
		{
			return (0.f);
		}

		Spectrum sampleF(const Vec3f &wo, Vec3f &wi, const Point2f &sample, Float &pdf, BxDFType *sampledType) const override
		{
			Float F = fresnelDielectric(cosTheta(wo), etaA, etaB);
			if (sample[0] < F)
			{
				wi = Vec3f(-wo.x, -wo.y, wo.z);
				if (sampledType)
					*sampledType = BxDFType(BSDF_SPECULAR | BSDF_REFLECTION);
				pdf = F;
				return F * r / std::abs(cosTheta(wi));
			}
			else
			{
				bool entering = cosTheta(wo) > 0;
				Float etaI = entering ? etaA : etaB;
				Float etaT = entering ? etaB : etaA;

				if (!refract(wo, faceForward(Normal(0, 0, 1), wo), etaI / etaT, wi))
					return 0;
				Spectrum ft = t * (1 - F);

				if (mode == TransportMode::Radiance)
					ft *= (etaI * etaI) / (etaT * etaT);
				if (sampledType)
					*sampledType = BxDFType(BSDF_SPECULAR | BSDF_TRANSMISSION);
				pdf = 1 - F;
				return ft / std::abs(cosTheta(wi));
			}
		}

		Float pdf(const Vec3f &wo, const Vec3f &wi) const override
		{
			return (0.f);
		}

	private:
		const Spectrum r;
		const Spectrum t;
		const Float etaA;
		const Float etaB;
		const FresnelDielectric fresnel;
		const TransportMode mode;
	};

	class LambertianReflection : public BxDF
	{
	public:
		LambertianReflection(const Spectrum &r)
			: BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), r(r)
		{}

		Spectrum f(const Vec3f &wo, const Vec3f &wi) const override
		{
			return (r / PI);
		}

		Spectrum rho(const Vec3f &wo, int nSamples, const Point2f *samples) const override
		{
			return (r);
		}

		Spectrum rho(int nSamples, const Point2f *samples1, const Point2f *samples2) const override
		{
			return (r);
		}

	private:
		const Spectrum r;
	};

	class OrenNayar : public BxDF
	{
	public:
		OrenNayar(const Spectrum &r, Float sigma)
			: BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), r(r)
		{
			sigma = radians(sigma);
			Float sigma2 = sigma * sigma;
			a = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
			b = 0.45f * sigma2 / (sigma2 + 0.09f);
		}

		Spectrum f(const Vec3f &wo, const Vec3f &wi) const override
		{
			Float sinThetaI = sinTheta(wi);
			Float sinThetaO = sinTheta(wo);
			
			Float maxCos = 0;
			if (sinThetaI > 1e-4 && sinThetaO > 1e-4)
			{
				Float sinPhiI = sinPhi(wi);
				Float cosPhiI = cosPhi(wi);
				Float sinPhiO = sinPhi(wo);
				Float cosPhiO = cosPhi(wo);
				Float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
				maxCos = std::max((Float)0, dCos);
			}

			Float sinAlpha, tanBeta;
			if (absCosTheta(wi) > absCosTheta(wo))
			{
				sinAlpha = sinThetaO;
				tanBeta = sinThetaI / absCosTheta(wi);
			}
			else
			{
				sinAlpha = sinThetaI;
				tanBeta = sinThetaO / absCosTheta(wo);
			}

			return (r / PI * (a + b * maxCos * sinAlpha * tanBeta));
		}

		Spectrum rho(const Vec3f &wo, int nSamples, const Point2f *samples) const override
		{
			return (r);
		}

		Spectrum rho(int nSamples, const Point2f *samples1, const Point2f *samples2) const override
		{
			return (r);
		}

	private:
		const Spectrum r;
		Float a;
		Float b;
	};

	class MicrofacetReflection : public BxDF
	{
	public:
		MicrofacetReflection(const Spectrum &r, const MicrofacetDistribution &distribution, const Fresnel &fresnel)
			: BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY))
			, r(r)
			, distribution(distribution)
			, fresnel(fresnel)
		{}

		Spectrum f(const Vec3f &wo, const Vec3f &wi) const override
		{
			Float cosThetaO = absCosTheta(wo);
			Float cosThetaI = absCosTheta(wi);
			Vec3f wh = wi + wo;
			if (cosThetaI == 0 || cosThetaO == 0)
				return (Spectrum(0.f));
			if (wh.x == 0 && wh.y == 0 && wh.z == 0)
				return (Spectrum(0.f));
			wh = normalize(wh);
			Spectrum f = fresnel.evaluate(dot(wi, wh));
			return (r * distribution.d(wh) * distribution.g(wo, wi) * f / (4 * cosThetaI * cosThetaO));
		}

		Spectrum sampleF(const Vec3f &wo, Vec3f &wi, const Point2f &sample, Float &pdf, BxDFType *sampledType = nullptr) const override
		{
			if (wo.z == 0)
				return 0.;
			Vec3f wh = distribution.sampleWh(wo, sample);
			if (dot(wo, wh) < 0)
				return 0.;
			wi = reflect(wo, wh);
			if (!sameHemisphere(wo, wi))
				return Spectrum(0.f);

			pdf = distribution.pdf(wo, wh) / (4 * dot(wo, wh));
			return (f(wo, wi));
		}

		Float pdf(const Vec3f &wi, const Vec3f &wo) const override
		{
			if (!sameHemisphere(wo, wi))
				return (0);
			Vec3f wh = normalize(wo + wi);
			return (distribution.pdf(wo, wh) / (4 * dot(wo, wh)));
		}

	private:
		const Spectrum r;
		const MicrofacetDistribution &distribution;
		const Fresnel &fresnel;
	};

	class MicrofacetTransmission : public BxDF
	{
	public:
		MicrofacetTransmission(const Spectrum &t, const MicrofacetDistribution &distribution, Float etaA, Float etaB, TransportMode mode)
			: BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY))
			, t(t)
			, distribution(distribution)
			, fresnel(etaA, etaB)
			, etaA(etaA), etaB(etaB)
			, mode(mode)
		{}

		Spectrum f(const Vec3f &wo, const Vec3f &wi) const override
		{
			if (sameHemisphere(wo, wi))
				return (0);

			Float cosThetaO = cosTheta(wo);
			Float cosThetaI = cosTheta(wi);
			if (cosThetaI == 0 || cosThetaO == 0)
				return (Spectrum(0));

			Float eta = cosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
			Vec3f wh = normalize(wo + wi * eta);
			if (wh.z < 0)
				wh = -wh;

			if (dot(wo, wh) * dot(wi, wh) > 0)
				return (Spectrum(0));

			Spectrum F = fresnel.evaluate(dot(wo, wh));

			Float sqrtDenom = dot(wo, wh) + eta * dot(wi, wh);
			Float factor = (mode == TransportMode::Radiance) ? (1 / eta) : 1;

			return (Spectrum(1.f) - F) * t *
				std::abs(distribution.d(wh) * distribution.g(wo, wi) * eta * eta *
					std::abs(dot(wi, wh)) * std::abs(dot(wo, wh)) * factor * factor /
					(cosThetaI * cosThetaO * sqrtDenom * sqrtDenom));
		}

		Spectrum sampleF(const Vec3f &wo, Vec3f &wi, const Point2f &sample, Float &pdf, BxDFType *sampledType = nullptr) const override
		{
			if (wo.z == 0)
				return (0.);
			Vec3f wh = distribution.sampleWh(wo, sample);
			if (dot(wo, wh) < 0)
				return (0.);

			Float eta = cosTheta(wo) > 0 ? (etaA / etaB) : (etaB / etaA);
			if (!refract(wo, (Normal)wh, eta, wi))
				return (0);
			pdf = this->pdf(wo, wi);
			return (f(wo, wi));
		}

		Float pdf(const Vec3f &wi, const Vec3f &wo) const override
		{
			if (sameHemisphere(wo, wi))
				return (0);

			Float eta = cosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
			Vec3f wh = normalize(wo + wi * eta);

			if (dot(wo, wh) * dot(wi, wh) > 0)
				return (0);

			Float sqrtDenom = dot(wo, wh) + eta * dot(wi, wh);
			Float dwh_dwi = std::abs((eta * eta * dot(wi, wh)) / (sqrtDenom * sqrtDenom));
			return distribution.pdf(wo, wh) * dwh_dwi;
		}

	private:
		const Spectrum t;
		const MicrofacetDistribution &distribution;
		const FresnelDielectric fresnel;
		const Float etaA;
		const Float etaB;
		const TransportMode mode;
	};

	class FresnelBlend : public BxDF
	{
	public:
		FresnelBlend(const Spectrum &rd, const Spectrum &rs, const MicrofacetDistribution &dist)
			: BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY))
			, rd(rd), rs(rs)
			, distribution(dist)
		{}

		Spectrum f(const Vec3f &wo, const Vec3f &wi) const override
		{
			auto pow5 = [](Float v) { return ((v * v) * (v * v) * v); };
			Spectrum diffuse = (28.f / (23.f * PI)) * rd * (Spectrum(1.f) - rs)
				* (1 - pow5(1 - 0.5f * std::abs(cosTheta(wi))))
				* (1 - pow5(1 - 0.5f * std::abs(cosTheta(wo))));
			Vec3f wh = wi + wo;
			if (wh.x == 0 && wh.y == 0 && wh.z == 0)
				return (Spectrum(0.f));
			wh = normalize(wh);
			Spectrum specular = distribution.d(wh) / (4 * std::abs(dot(wi, wh))
				* std::max(std::abs(cosTheta(wi)), std::abs(cosTheta(wo))))
				* schlickFresnel(dot(wi, wh));
			return (diffuse + specular);
		}

		Spectrum sampleF(const Vec3f &wo, Vec3f &wi, const Point2f &sample, Float &pdf, BxDFType *sampledType = nullptr) const override
		{
			Point2f u = sample;
			if (u[0] < .5)
			{
				u[0] = std::min(2 * u[0], OneMinusEpsilon);
				wi = cosineSampleHemisphere(u);
				if (wo.z < 0)
					wi.z *= -1;
			}
			else
			{
				u[0] = std::min(2 * (u[0] - .5f), OneMinusEpsilon);
				Vec3f wh = distribution.sampleWh(wo, u);
				wi = reflect(wo, wh);
				if (!sameHemisphere(wo, wi))
					return Spectrum(0.f);
			}
			pdf = this->pdf(wo, wi);
			return (f(wo, wi));
		}

		Float pdf(const Vec3f &wi, const Vec3f &wo) const override
		{
			if (!sameHemisphere(wo, wi))
				return (0);
			Vec3f wh = normalize(wo + wi);
			Float pdfWh = distribution.pdf(wo, wh);
			return (.5f * (std::abs(cosTheta(wi)) / PI + pdfWh / (4 * dot(wo, wh))));
		}

		Spectrum schlickFresnel(Float cosTheta) const
		{
			auto pow5 = [](Float v) { return ((v * v) * (v * v) * v); };
			return (rs + pow5(1 - cosTheta) * (Spectrum(1.) - rs));
		}

	private:
		const Spectrum rd;
		const Spectrum rs;
		const MicrofacetDistribution &distribution;
	};
}