#pragma once

#include "Atlas/Atlas.h"
#include "Atlas/core/RgbSpectrum.h"
#include "Atlas/core/Vectors.h"

namespace atlas
{
	namespace sh
	{
		struct BSDF
		{
			Spectrum color;
			Vec3f wi;
			Float pdf;
		};
	}
}