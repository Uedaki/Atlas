#pragma once

# ifdef ATLAS_EXPORT
#   define ATLAS  __declspec( dllexport )
# else
#   define ATLAS __declspec( dllimport )
# endif

#include <glm/glm.hpp>

#include <vector>

namespace atlas
{
	struct Buffer
	{
		std::vector<glm::vec3> image;
		std::vector<glm::vec3> albedo;
		std::vector<glm::vec3> normal;

		ATLAS Buffer() = default;
		ATLAS Buffer(uint32_t size);

		ATLAS void resize(uint32_t size);
	};
}