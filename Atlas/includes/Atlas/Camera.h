#pragma once

# ifdef ATLAS_EXPORT
#   define ATLAS  __declspec( dllexport )
# else
#   define ATLAS __declspec( dllimport )
# endif

#include <glm/glm.hpp>

#include <cstdint>

#include "Simd.h"

namespace atlas
{
	namespace rendering { struct Ray; };

	class Camera
	{
	public:
		ATLAS Camera(const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up, float fov, float aspect);

		ATLAS void enableDefocusBlur(float aperture, float focusDistance);
		ATLAS void reset();

		rendering::Ray getRay(const float x, const float y) const;
		SimdRay getSimdRay(const float4 x, const float4 y) const;
	private:
		glm::vec3 pos;
		glm::vec3 target;
		glm::vec3 up;

		float fov;
		float aspect;

		bool isBlurEnable;
		float aperture;
		float lensRadius;
		float focusDistance;

		glm::vec3 u;
		glm::vec3 v;
		glm::vec3 w;

		glm::vec3 corner;
		glm::vec3 horizontal;
		glm::vec3 vertical;
	};
}