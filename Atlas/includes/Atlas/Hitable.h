#pragma once

#include "Atlas/Bound.h"

#include "Simd.h"

namespace atlas
{
	namespace rendering { struct HitRecord; };
	namespace rendering { struct Ray; };

	class Hitable
	{
	public:
		virtual bool hit(const rendering::Ray &ray, const float min, const float max, rendering::HitRecord &record) const = 0;
		virtual bool4 simdHit(SimdRay &ray, float4 min, float4 max, SimdHitRecord &record) const { return (bool4(0.f)); };

		inline const Bound &getBound() const { return (bound); }
		inline void setBound(const Bound &value) { bound = value; }
	protected:
		Bound bound;
	};
}