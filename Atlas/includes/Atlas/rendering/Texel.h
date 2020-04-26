#pragma once

#include <glm/glm.hpp>

namespace atlas
{
	namespace rendering
	{
		struct Texel
		{
			glm::vec3 color = glm::vec3(0.f);
			glm::vec3 albedo = glm::vec3(0.f);
			glm::vec3 normal = glm::vec3(0.f);

			void operator+=(const Texel &texel)
			{
				color += texel.color;
				albedo += texel.albedo;
				normal += texel.normal;
			}
		};
	}
}
