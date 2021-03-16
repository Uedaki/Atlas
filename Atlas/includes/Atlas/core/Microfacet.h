#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Points.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
	class MicrofacetDistribution
	{
	public:
		virtual Float d(const Vec3f &wh) const = 0;
		virtual Float lambda(const Vec3f &w) const = 0;

		Float g1(const Vec3f &w) const
		{
			return (1.f / (1.f + lambda(w)));
		}

		Float g(const Vec3f &wo, const Vec3f &wi) const
		{
			return (1.f / (1 + lambda(wo) + lambda(wi)));
		}

		virtual Vec3f sampleWh(const Vec3f &wo, const Point2f &u) const = 0;

		Float pdf(const Vec3f &wo, const Vec3f &wh) const;

	protected:
		MicrofacetDistribution(bool sampleVisibleArea)
			: sampleVisibleArea(sampleVisibleArea)
		{}

		const bool sampleVisibleArea;
	};

	class BeckmannDistribution : public MicrofacetDistribution
	{
	public:
		static inline Float roughnessToAlpha(Float roughness)
		{
			roughness = std::max(roughness, (Float)1e-3);
			Float x = std::log(roughness);
			return 1.62142f + 0.819955f * x + 0.1734f * x * x +
				0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
		}

		BeckmannDistribution(Float alphaX, Float alphaY, bool sampleVis = true)
			: MicrofacetDistribution(sampleVis)
			, alphaX(std::max(Float(0.001), alphaX))
			, alphaY(std::max(Float(0.001), alphaY))
		{}

		ATLAS Float d(const Vec3f &wh) const override;
		ATLAS Vec3f sampleWh(const Vec3f &wo, const Point2f &u) const override;

	private:
		ATLAS Float lambda(const Vec3f &w) const override;

		const Float alphaX;
		const Float alphaY;
	};

	class TrowbridgeReitzDistribution : public MicrofacetDistribution
	{
	public:
		static inline Float RoughnessToAlpha(Float roughness)
		{
			roughness = std::max(roughness, (Float)1e-3);
			Float x = std::log(roughness);
			return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
				0.000640711f * x * x * x * x;
		}

		TrowbridgeReitzDistribution(Float alphaX, Float alphaY, bool samplevis = true)
			: MicrofacetDistribution(samplevis)
			, alphaX(std::max(Float(0.001), alphaX))
			, alphaY(std::max(Float(0.001), alphaY))
		{}

		ATLAS Float d(const Vec3f &wh) const override;
		ATLAS Vec3f sampleWh(const Vec3f &wo, const Point2f &u) const override;

	private:
		ATLAS Float lambda(const Vec3f &w) const;

		const Float alphaX;
		const Float alphaY;
	};
}