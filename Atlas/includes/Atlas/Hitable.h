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

		inline const Bound &getBound() const { return (bound); }
		inline void setBound(const Bound &value) { bound = value; }
	protected:
		Bound bound;
	};
}