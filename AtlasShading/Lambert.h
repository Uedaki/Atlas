#pragma once

#include "AtlasShadingLibHeader.h"
#include "BSDFShader.h"

namespace atlas
{
	namespace sh
	{
		class Material;

		struct Lambert : public BSDFShader
		{
			ATLAS_SH Spectrum f(const Vec3f &wo, const Vec3f &wi, const std::vector<uint8_t> &data) const override;

			ShadingInput<Spectrum> r;
		};

		ATLAS_SH std::shared_ptr<Material> createLambertMaterial(const Spectrum &r);
	}
}