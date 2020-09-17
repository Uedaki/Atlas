#pragma once

#include <array>
#include <vector>

#include "Atlas/Hitable.h"

#include "atlas/Simd.h"

# ifdef ATLAS_EXPORT
#   define ATLAS  __declspec( dllexport )
# else
#   define ATLAS __declspec( dllimport )
# endif

#define SIMD

namespace atlas
{
	namespace rendering
	{
		struct Ray;

		class Acceleration : public Hitable
		{
		public:
			Acceleration() = default;
			ATLAS Acceleration(std::vector<const Hitable *> &source, const Bound &bound, std::vector<Acceleration> &pool);
			ATLAS void feed(std::vector<const Hitable *> &source, const Bound &bound, std::vector<Acceleration> &pool);
			
			ATLAS bool hit(const Ray &ray, const float min, const float max, HitRecord &record) const override;

		private:
			std::array<const Hitable *, 4> elements = { nullptr };
			
#ifndef SIMD
			std::array<Bound, 4> bounds;
#endif

			ATLAS void split(std::vector<const Hitable *> &src,
				std::vector<const Hitable *> &nearElements, Bound &nearBound,
				std::vector<const Hitable *> &farElements, Bound &farBound,
				int axis);

#ifdef SIMD
			bool4 simdIntersect(const Ray &ray, const float min, const float max) const;

			float4 minBoundX;
			float4 minBoundY;
			float4 minBoundZ;

			float4 maxBoundX;
			float4 maxBoundY;
			float4 maxBoundZ;
#endif
		};
	}
}