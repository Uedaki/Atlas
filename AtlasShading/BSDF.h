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
		Vec3f wi = Vec3f(0, 0, 0);
		Float pdf = 0;
		Float scatteringPdf = 0;
	};
}