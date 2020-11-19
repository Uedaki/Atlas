#pragma once

# ifdef ACHERON_EXPORTS
#   define ACH  __declspec( dllexport )
# else
#   define ACH __declspec( dllimport )
# endif


#include <glm/glm.hpp>

#include "../Atlas/includes/Atlas/Simd.h"
#include "../Atlas/includes/Atlas/Bound.h"

#include "BoundingCone.h"

namespace atlas
{
	namespace rendering {
		struct Ray;
	}
}

class Bound
{
public:
	Bound() = default;
	ACH Bound(const glm::vec3 &min, const glm::vec3 &max);

	ACH void operator=(const Bound &bound);
	ACH Bound operator+(const Bound &bound);
	ACH void operator+=(const Bound &bound);
	ACH void operator=(const atlas::Bound &bound);
	ACH Bound operator+(const atlas::Bound &bound);
	ACH void operator+=(const atlas::Bound &bound);

	ACH bool intersect(const atlas::rendering::Ray &ray, float min, float max) const;

	ACH bool intersectCone(const BoundingCone &cone, float min, float max) const;
	ACH void ComputeBoxHeightInterval(const BoundingCone &cone, float &boxMinHeight, float &boxMaxHeight) const;
	
	ACH bool4 simdIntersect(SimdRay &ray, float4 min, float4 max) const;

	ACH int largestAxis() const;
	ACH glm::vec3 getCenter() const;

	inline const glm::vec3 getMin() const {
		return (min);
	}
	inline const glm::vec3 getMax() const {
		return (max);
	}

	inline float length() const {
		return (glm::length(max - min));
	}
	inline float length(int axis) const {
		return (max[axis] - min[axis]);
	}

private:
	glm::vec3 max = glm::vec3(0);
	glm::vec3 min = glm::vec3(0);
};