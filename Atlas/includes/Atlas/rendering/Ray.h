#pragma once

#include <glm/glm.hpp>

namespace atlas
{
	namespace rendering
	{
		struct Ray
		{
			glm::vec3 origin;
			glm::vec3 dir;

			glm::vec3 invDir;
			int sign[3];

			Ray() = default;
			Ray(const glm::vec3 &o, const glm::vec3 &d)
				: origin(o), dir(glm::normalize(d))
			{
				invDir = 1.f / dir;
				sign[0] = (invDir.x < 0);
				sign[1] = (invDir.y < 0);
				sign[2] = (invDir.z < 0);
			}
		};
	}
}