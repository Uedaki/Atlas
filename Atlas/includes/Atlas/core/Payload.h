#pragma once

#include "atlas/Atlas.h"
#include "atlas/core/Batch.h"
#include "atlas/core/Points.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
	struct BoundingCone
	{
		Point3f origin;
		Vec3f dir;
		Float dot = 1.f;
		Float tmin = std::numeric_limits<float>::max();
		mutable float tmax = std::numeric_limits<float>::min();
	};

	struct Payload
	{

		BoundingCone cone;
		Batch *batch;
		uint32_t first;
		uint32_t size;
	};
}