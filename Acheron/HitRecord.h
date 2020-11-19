#pragma once

#include "../Atlas/includes/Atlas/Material.h"

struct HitRecord
{
	bool hit = false;
	float t = std::numeric_limits<float>::max();
	glm::vec3 p = glm::vec3(-1, -1, -1);
	glm::vec2 uv = glm::vec2(0, 0);
	glm::vec3 normal = glm::vec3(-1, -1, -1);
	const atlas::Material *material = nullptr;

	void reset()
	{
		hit = false;
		t = std::numeric_limits<float>::max();
		p = glm::vec3(-1, -1, -1);
		uv = glm::vec2(0, 0);
		normal = glm::vec3(-1, -1, -1);
		material = nullptr;
	}
};