#pragma once

#include "atlas/core/BxDF.h"
#include "atlas/core/Interaction.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
	class BSDF
	{
	public:
		BSDF(const SurfaceInteraction &si, Float eta = 1)
			: eta(eta), ns(si.shading.n), ng(si.n),
			ss(normalize(si.shading.dpdu)), ts(cross(ns, ss))
		{}

		void add(BxDF *b)
		{
			DCHECK(nBxDFs < MaxBxDFs);
			bxdfs[nBxDFs] = b;
			nBxDFs++;
		}

		int numComponents(BxDFType flags = BSDF_ALL) const
		{
			int num = 0;
			for (int i = 0; i < nBxDFs; i++)
			{
				if (bxdfs[i]->matchesFlags(flags))
					num++;
			}
			return (num);
		}

		Vec3f worldToLocal(const Vec3f &v) const
		{
			return (Vec3f(dot(v, ss), dot(v, ts), dot(v, ns)));
		}

		Vec3f localToWorld(const Vec3f &v) const
		{
			return (Vec3f(ss.x * v.x + ts.x * v.y + ns.x * v.z,
				ss.y * v.x + ts.y * v.y + ns.y * v.z,
				ss.z * v.x + ts.z * v.y + ns.z * v.z));
		}

		Spectrum f(const Vec3f &woW, const Vec3f &wiW, BxDFType flags) const
		{
			Vec3f wi = worldToLocal(wiW);
			Vec3f wo = worldToLocal(woW);
			bool reflect = dot(wiW, ng) * dot(woW, ng) > 0;
			Spectrum f(0.f);
			for (int i = 0; i < nBxDFs; i++)
			{
				if (bxdfs[i]->matchesFlags(flags)
					&& ((reflect && (bxdfs[i]->type & BSDF_REFLECTION))
						|| (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
					f += bxdfs[i]->f(wo, wi);
			}
			return (f);
		}

		Spectrum rho(int nSamples, const Point2f *samples1, const Point2f *samples2, BxDFType flags) const
		{
			Spectrum f(0.f);
			for (int i = 0; i < nBxDFs; i++)
			{
				if (bxdfs[i]->matchesFlags(flags))
					f += bxdfs[i]->rho(nSamples, samples1, samples2);
			}
			return (f);
		}

		Spectrum rho(const Vec3f &woW, int nSamples, const Point2f *samples, BxDFType flags) const
		{
			Vec3f wo = worldToLocal(woW);
			Spectrum f(0.f);
			for (int i = 0; i < nBxDFs; i++)
			{
				if (bxdfs[i]->matchesFlags(flags))
					f += bxdfs[i]->rho(wo, nSamples, samples);
			}
			return (f);
		}

		Spectrum sampleF(const Vec3f &woWorld, Vec3f &wiWorld, const Point2f &u, Float &pdf, BxDFType type = BSDF_ALL, BxDFType *sampledType = nullptr) const
		{
			int matchingComps = numComponents(type);
			if (matchingComps == 0)
			{
				pdf = 0;
				if (sampledType)
					*sampledType = BxDFType(0);
				return Spectrum(0);
			}

			int comp = std::min((int)std::floor(u[0] * matchingComps), matchingComps - 1);

			BxDF *bxdf = nullptr;
			int count = comp;
			for (int i = 0; i < nBxDFs; ++i)
			{
				if (bxdfs[i]->matchesFlags(type) && count-- == 0)
				{
					bxdf = bxdfs[i];
					break;
				}
			}

			Point2f uRemapped(std::min(u[0] * matchingComps - comp, OneMinusEpsilon), u[1]);

			Vec3f wi;
			Vec3f wo = worldToLocal(woWorld);
			if (wo.z == 0)
				return 0.;
			pdf = 0;
			if (sampledType)
				*sampledType = bxdf->type;
			Spectrum f = bxdf->sampleF(wo, wi, uRemapped, pdf, sampledType);
			if (pdf == 0)
			{
				if (sampledType)
					*sampledType = BxDFType(0);
				return 0;
			}
			wiWorld = localToWorld(wi);

			if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
			{
				for (int i = 0; i < nBxDFs; ++i)
				{
					if (bxdfs[i] != bxdf && bxdfs[i]->matchesFlags(type))
						pdf += bxdfs[i]->pdf(wo, wi);
				}
			}
			if (matchingComps > 1)
				pdf /= matchingComps;

			if (!(bxdf->type & BSDF_SPECULAR))
			{
				bool reflect = dot(wiWorld, ng) * dot(woWorld, ng) > 0;
				f = 0.;
				for (int i = 0; i < nBxDFs; ++i)
				{
					if (bxdfs[i]->matchesFlags(type) &&
						((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
							(!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
						f += bxdfs[i]->f(wo, wi);
				}
			}
			return f;
		}

		const Float eta;

	private:
		const Normal ns;
		const Normal ng;
		const Vec3f ss;
		const Vec3f ts;
		
		int nBxDFs = 0;
		static constexpr int MaxBxDFs = 8;
		BxDF *bxdfs[MaxBxDFs];

		friend class MixMaterial;
	};
}