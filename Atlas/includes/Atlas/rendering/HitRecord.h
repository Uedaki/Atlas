#pragma once

#include <glm/glm.hpp>

namespace atlas
{
	class Material;

	namespace rendering
	{
		struct HitRecord
		{
			float t = -1;
			glm::vec3 p = glm::vec3(-1, -1, -1);
			glm::vec2 uv = glm::vec2(0, 0);
			glm::vec3 normal = glm::vec3(-1, -1, -1);
			const Material *material = nullptr;
		};
	}
}