#pragma once

#include "AtlasShadingLibHeader.h"
#include "BSDFShader.h"

namespace atlas
{
	class Material;

	struct DisneyPrincipled : public BSDFShader
	{
		ATLAS_SH void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const override;
		ATLAS_SH Spectrum f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const override;

		ATLAS_SH Spectrum diffuseModel(const Spectrum &baseColor, const Float roughness, const Float nDotL, const Float nDotV, const Float lDotH) const;
		ATLAS_SH Spectrum subsurfaceModel(const Spectrum &baseColor, const Float roughness, const Float nDotL, const Float nDotV, const Float lDotH) const;

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
