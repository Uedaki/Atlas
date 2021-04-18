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
			Spectrum Li = BLACK;
			Spectrum Le = BLACK;
			Vec3f wi = Vec3f(0, 0, 0);
			Float pdf = 0;
		};
	}
}