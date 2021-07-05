#pragma once

#include "Atlas/Atlas.h"
#include "Atlas/core/RgbSpectrum.h"
#include "Atlas/core/Vectors.h"

namespace atlas
{
	struct BSDF
	{
		Spectrum Li = BLACK;
		Spectrum Le = BLACK;
		Float pdf = 0;
		Float scatteringPdf = 0;
	};

	struct BSDFSample : BSDF
	{
		Vec3f wi;
	};
}