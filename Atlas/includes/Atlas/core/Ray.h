#pragma once

#include <array>

#include "atlas/Atlas.h"
#include "atlas/core/Points.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
	class Medium;

	struct Ray
	{
		Point3f origin;
		Vec3f dir;
		mutable Float tmax = std::numeric_limits<Float>::max();
		Float time;
		const Medium *medium;

		Ray()
			: tmax(INFINITY), time(0.f), medium(nullptr)
		{}

		Ray(const Point3f &o, const Vec3f &d, Float tmax = INFINITY, Float time = 0.f, const Medium *medium = nullptr)
			: origin(o), dir(normalize(d)), tmax(tmax), time(time), medium(medium)
		{}

		Point3f operator()(Float t) const
		{
			return (origin + dir * t);
		}
	};

	struct RayDifferential : public Ray
	{
		bool hasDifferentials;
		Point3f rxOrigin;
		Point3f ryOrigin;
		Vec3f rxDirection;
		Vec3f ryDirection;

		RayDifferential() {
			hasDifferentials = false;
		}
		RayDifferential(const Point3f &o, const Vec3f &d, Float tMax = INFINITY,
			Float time = 0.f, const Medium *medium = nullptr)
			: Ray(o, d, tMax, time, medium)
		{
			hasDifferentials = false;
		}
		
		RayDifferential(const Ray &ray) : Ray(ray)
		{
			hasDifferentials = false;
		}

		void scaleDifferentials(Float s) {
			rxOrigin = origin + (rxOrigin - origin) * s;
			ryOrigin = origin + (ryOrigin - origin) * s;
			rxDirection = dir + (rxDirection - dir) * s;
			ryDirection = dir + (ryDirection - dir) * s;
		}
	};

	struct ConeRay
	{
		Point3f origin;
		Vec3f dir;
		Float cos = 1;
		Float tmin = std::numeric_limits<Float>::min();
		mutable Float tmax = std::numeric_limits<Float>::max();
		
		uint32_t nbrRays = 0;
		Ray *rays = nullptr;
	
		ConeRay() = default;
		ConeRay(const Point3f &o, const Vec3f &d, Float cos, Float tmin, Float tmax)
			: origin(o), dir(d), cos(cos), tmin(tmin), tmax(tmax)
		{}
	};
}