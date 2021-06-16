#pragma once

#include "AtlasShadingLibHeader.h"
#include "BSDFShader.h"

namespace atlas
{
	class Material;

	struct Lambert : public BSDFShader
	{
		ATLAS_SH Spectrum f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const override;

		ShadingInput<Spectrum> iR;
	};

	ATLAS_SH std::shared_ptr<Material> createLambertMaterial(const Spectrum &r);
}