#pragma once

#include "AtlasShadingLibHeader.h"
#include "BSDFShader.h"

namespace atlas
{
	namespace sh
	{
		class Material;

		struct Glass : public BSDFShader
		{
			ATLAS_SH void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, std::vector<uint8_t> &data) const override;
			ATLAS_SH Spectrum f(const Vec3f &wo, const Vec3f &wi, const std::vector<uint8_t> &data) const override;

			Float eta;
		};

		ATLAS_SH std::shared_ptr<Material> createGlassMaterial(Float eta);
	}
}