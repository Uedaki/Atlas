#pragma once

# ifdef ATLAS_EXPORT
#   define ATLAS  __declspec( dllexport )
# else
#   define ATLAS __declspec( dllimport )
# endif

#include <glm/glm.hpp>

#include "Simd.h"

namespace atlas
{
	namespace rendering { struct Ray; };

	class Bound
	{
	public:
		Bound() = default;
		ATLAS Bound(const glm::vec3 &min, const glm::vec3 &max);

		ATLAS void operator=(const Bound &bound);
		ATLAS Bound operator+(const Bound &bound);
		ATLAS void operator+=(const Bound &bound);

		ATLAS bool intersect(const rendering::Ray &ray, float min, float max) const;
		ATLAS bool4 simdIntersect(SimdRay &ray, float4 min, float4 max) const;

		ATLAS int largestAxis() const;
		ATLAS glm::vec3 getCenter() const;

		inline const glm::vec3 getMin() const { return (min); }
		inline const glm::vec3 getMax() const { return (max); }

		inline float length() const { return (glm::length(max - min)); }
		inline float length(int axis) const { return (max[axis] - min[axis]); }

	private:
		glm::vec3 max = glm::vec3(-1);
		glm::vec3 min = glm::vec3(1);
	};
}