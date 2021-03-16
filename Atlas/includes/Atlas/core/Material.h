#pragma once

#include "atlas/Atlas.h"
#include "atlas/core/BSDF.h"
#include "atlas/core/Microfacet.h"
#include "atlas/core/Interaction.h"
#include "atlas/core/Texture.h"

namespace atlas
{
	class Material
	{
	public:
		virtual ~Material() = default;

		virtual void computeScatteringFunctions(SurfaceInteraction &si, TransportMode mode, bool allowMultipleLobes) const = 0;

		static void bump(const std::shared_ptr<Texture<Float>> &d, SurfaceInteraction &si)
		{

		}
	};

	class MatteMaterial : public Material
	{
	public:
		struct Info
		{
			std::shared_ptr<Texture<Spectrum>> kd;
			std::shared_ptr<Texture<Float>> sigma;
			std::shared_ptr<Texture<Float>> bumpMap;
		};

		static std::shared_ptr<Material> create(const Info &info = Info())
		{
			Info ci;
			ci.kd = info.kd ? info.kd : atlas::createSpectrumConstant(0.5f);
			ci.sigma = info.sigma ? info.sigma : atlas::createFloatConstant(0.f);
			ci.bumpMap = info.bumpMap;
			return (std::make_shared<MatteMaterial>(ci));
		}

		MatteMaterial(const Info &info)
			: kd(info.kd), sigma(info.sigma), bumpMap(info.bumpMap)
		{}

		MatteMaterial(const std::shared_ptr<Texture<Spectrum>> &kd, const std::shared_ptr<Texture<Float>> &sigma, const std::shared_ptr<Texture<Float>> &bumpMap)
			: kd(kd), sigma(sigma), bumpMap(bumpMap)
		{}

		void computeScatteringFunctions(SurfaceInteraction &si, TransportMode mode, bool allowMultipleLobes) const override
		{
			if (bumpMap)
				bump(bumpMap, si);

			si.bsdf = new BSDF(si);
			Spectrum r = kd->evaluate(si);
			Float sig = clamp(sigma->evaluate(si), 0.f, 90.f);
			if (!r.isBlack())
			{
				if (sig == 0)
					si.bsdf->add(new LambertianReflection(r));
				else
					si.bsdf->add(new OrenNayar(r, sig));
			}
		}

	private:
		std::shared_ptr<Texture<Spectrum>> kd;
		std::shared_ptr<Texture<Float>> sigma;
		std::shared_ptr<Texture<Float>> bumpMap;
	};

	typedef MatteMaterial::Info MatteMaterialInfo;

	class PlasticMaterial : public Material
	{
	public:
		PlasticMaterial(const std::shared_ptr<Texture<Spectrum>> &kd, const std::shared_ptr<Texture<Spectrum>> &ks,
			const std::shared_ptr<Texture<Float>> &roughness, const std::shared_ptr<Texture<Float>> &bumpMap,
			bool remapRoughness)
			: kd(kd), ks(ks), roughness(roughness), bumpMap(bumpMap), remapRoughness(remapRoughness)
		{}

		void computeScatteringFunctions(SurfaceInteraction &si, TransportMode mode, bool allowMultipleLobes) const override
		{
			if (bumpMap)
				bump(bumpMap, si);

			si.bsdf = new BSDF(si);
			Spectrum r = kd->evaluate(si);
			if (!r.isBlack())
			{
				si.bsdf->add(new LambertianReflection(r));
			}

			Spectrum s = ks->evaluate(si);
			if (!s.isBlack())
			{
				Fresnel *fresnel = new FresnelDielectric(1.f, 1.5f);
				Float rough = roughness->evaluate(si);
				if (remapRoughness)
					rough = TrowbridgeReitzDistribution::RoughnessToAlpha(rough);
				MicrofacetDistribution *distrib = new TrowbridgeReitzDistribution(rough, rough);
				si.bsdf->add(new MicrofacetReflection(s, *distrib, *fresnel));
			}
		}

	private:
		std::shared_ptr<Texture<Spectrum>> kd;
		std::shared_ptr<Texture<Spectrum>> ks;
		std::shared_ptr<Texture<Float>> roughness;
		std::shared_ptr<Texture<Float>> bumpMap;
		const bool remapRoughness;
	};

	class MixMaterial : public Material
	{
	public:
		MixMaterial(const std::shared_ptr<Material> &m1, const std::shared_ptr<Material> &m2, const std::shared_ptr<Texture<Spectrum>> &scale)
			: m1(m1), m2(m2), scale(scale)
		{}

		void computeScatteringFunctions(SurfaceInteraction &si, TransportMode mode, bool allowMultipleLobes) const override
		{
			Spectrum s1 = scale->evaluate(si);
			Spectrum s2 = (Spectrum(1.f) - s1);
			SurfaceInteraction &si2 = si;

			m1->computeScatteringFunctions(si, mode, allowMultipleLobes);
			m2->computeScatteringFunctions(si2, mode, allowMultipleLobes);

			int n1 = si.bsdf->numComponents();
			int n2 = si2.bsdf->numComponents();

			for (int i = 0; i < n1; i++)
			{
				si.bsdf->bxdfs[i] = new ScaledBxDF(si.bsdf->bxdfs[i], s1);
			}

			for (int i = 0; i < n2; i++)
			{
				si.bsdf->add(new ScaledBxDF(si2.bsdf->bxdfs[i], s2));
			}
		}

	private:
		std::shared_ptr<Material> m1;
		std::shared_ptr<Material> m2;
		std::shared_ptr<Texture<Spectrum>> scale;
	};
}