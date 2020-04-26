#pragma once

#include <array>
#include <vector>

#include "Atlas/Hitable.h"

#include "atlas/Simd.h"

namespace atlas
{
	namespace rendering
	{
		struct Ray;

		class Acceleration : public Hitable
		{
		public:
			Acceleration() = default;
			Acceleration(std::vector<const Hitable *> &source, const Bound &bound, std::vector<Acceleration> &pool);
			void feed(std::vector<const Hitable *> &source, const Bound &bound, std::vector<Acceleration> &pool);
			bool hit(const Ray &ray, const float min, const float max, HitRecord &record) const override;
			bool4 simdHit(SimdRay &ray, float4 min, float4 max, SimdHitRecord &record) const override;

		private:
			std::array<const Hitable *, 2> elements = { nullptr };
		};
	}
}