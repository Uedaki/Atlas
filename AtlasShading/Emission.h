#pragma once

#include "atlas/atlas.h"
#include "atlas/core/RgbSpectrum.h"

#include "BSDFShader.h"
#include "ShadingIO.h"

namespace atlas
{
	namespace sh
	{
		struct Emission : public BSDFShader
		{
			void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const override
			{
				BSDF bsdf = { 0 };
				bsdf.Le = iColor.get(block) * iStrength.get(block);
				out.set(block, bsdf);
			}

			ShadingInput<Spectrum> iColor;
			ShadingInput<Float> iStrength;
		};
	}
}