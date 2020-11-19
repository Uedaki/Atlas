#pragma once

#include <glm/glm.hpp>

struct BoundingCone
{
	glm::vec3 origin = glm::vec3(0.f);
	glm::vec3 dir = glm::vec3(0.f);
	float dot = 1.f;
	float tmin = std::numeric_limits<float>::max();
	float tmax = std::numeric_limits<float>::min();
};