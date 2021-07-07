#pragma once

#include "atlas/atlas.h"
#include "atlas/core/RgbSpectrum.h"

#include "BSDFShader.h"
#include "ShadingIO.h"

namespace atlas
{
	struct Emission : public BSDFShader
	{
		void evaluateBsdf(const Vec3f &wo, const Vec3f &wi, const SurfaceInteraction &si, const DataBlock &block, BSDF &bsdf) const override
		{
			bsdf.Le = iColor.get(block) * iStrength.get(block);
		}

		ShadingInput<Spectrum> iColor;
		ShadingInput<Float> iStrength;
	};
}