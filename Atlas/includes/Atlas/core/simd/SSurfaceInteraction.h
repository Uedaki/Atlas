#pragma once

#include "atlas/Atlas.h"
#include "atlas/core/simd/Simd.h"
#include "atlas/core/simd/SPoints.h"
#include "atlas/core/simd/SVectors.h"

namespace atlas
{
	struct S4SurfaceInteraction
	{
		S4Float t;
		S4Point3 p;
		S4Normal normal;
	};
}