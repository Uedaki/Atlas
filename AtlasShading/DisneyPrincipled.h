#pragma once

#include "AtlasShadingLibHeader.h"
#include "BSDFShader.h"

namespace atlas
{
	class Material;

	struct DisneyPrincipled : public BSDFShader
	{
		ATLAS_SH void evaluateBsdf(const Vec3f &wo, const Vec3f &wi, const SurfaceInteraction &si, const DataBlock &block, BSDF &bsdf) const override;
		ATLAS_SH void evaluateWi(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, const DataBlock &block, Vec3f &wi) const;
		ATLAS_SH Spectrum f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const override;

		ATLAS_SH Spectrum diffuseModel(const DataBlock &block, const Float nDotL, const Float nDotV, const Float lDotH) const;
		ATLAS_SH Spectrum specularModel(const DataBlock &block, const Vec3f &wo, const Vec3f &wi, const Vec3f &tangent, const Vec3f &binormal, const Vec3f &h, const Float nDotL, const Float nDotV, const Float nDotH, const Float lDotH) const;

		Float scatteringPdf(const Interaction &intr, const Vec3f &wo, const Vec3f &wi) const override
		{
			return (1);
		}

		ShadingInput<Spectrum> iBaseColor;
		ShadingInput<Float> iSubsurface;
		ShadingInput<Float> iMetallic;
		ShadingInput<Float> iSpecular;
		ShadingInput<Float> iSpecularTint;
		ShadingInput<Float> iRoughness;
		ShadingInput<Float> iAnisotropic;
		ShadingInput<Float> iSheen;
		ShadingInput<Float> iSheenTint;
		ShadingInput<Float> iClearcoat;
		ShadingInput<Float> iClearcoatGloss;
	};
}
