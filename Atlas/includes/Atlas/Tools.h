#pragma once

# ifdef ATLAS_EXPORT
#   define ATLAS  __declspec( dllexport )
# else
#   define ATLAS __declspec( dllimport )
# endif

#include <glm/glm.hpp>

namespace atlas
{
	class Tools
	{
	public:
		ATLAS static float rand();
		ATLAS static glm::vec3 randomInUnitSphere();
		ATLAS static glm::vec3 randomUniformHemisphere();
		ATLAS static glm::vec3 randomCosineHemisphere();
		ATLAS static bool refract(const glm::vec3 &v, const glm::vec3 &n, const float ni_over_nt, glm::vec3 &refracted);
		ATLAS static float schlick(const float cosine, const float ri);
		ATLAS static glm::vec3 reflect(const glm::vec3 &v, const glm::vec3 &n);
		ATLAS static glm::vec3 randomInUnitDisk();
	};
}