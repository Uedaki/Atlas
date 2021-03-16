#pragma once

#ifdef _USE_SIMD

#include "atlas/core/simd/Simd.h"
#include "atlas/core/simd/SPoints.h"
#include "atlas/core/simd/SVectors.h"

namespace atlas
{
	struct S4Ray
	{
		S4Point3 origin;
		S4Vec3 dir;
		mutable S4Float tmax;

		S4Ray() = default;
		S4Ray(const S4Point3 &o, const S4Vec3 &d, Float tmax = std::numeric_limits<Float>::max())
			: origin(o), dir(d), tmax(tmax)
		{}
		S4Ray(const S4Point3 &o, const S4Vec3 &d, S4Float tmax = S4Float(std::numeric_limits<Float>::max()))
			: origin(o), dir(d), tmax(tmax)
		{}

		S4Point3 operator()(S4Float t) const
		{
			return (origin + dir * t);
		}
	};

	struct S4ConeRay
	{
		S4Point3 origin;
		S4Vec3 dir;
		S4Float cos;
		S4Float tmin;
		mutable S4Float tmax;

		uint32_t maxNbrRays = 0;
		S4Ray *rays = nullptr;

		S4ConeRay() = default;
		S4ConeRay(const S4Point3 &o, const S4Vec3 &d, S4Float cos, S4Float tmin, S4Float tmax)
			: origin(o), dir(d), cos(cos), tmin(tmin), tmax(tmax)
		{}
	};
}

#endif