#pragma once

#include "AtlasShadingLibHeader.h"
#include "BSDFShader.h"

namespace atlas
{
	class Material;

	struct Glass : public BSDFShader
	{
		ATLAS_SH void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const override;
		ATLAS_SH Spectrum f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const override;

		ShadingInput<Float> iEta;
	};

	ATLAS_SH std::shared_ptr<Material> createGlassMaterial(Float eta);
}