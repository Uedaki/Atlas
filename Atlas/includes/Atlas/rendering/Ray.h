#pragma once

#include <glm/glm.hpp>
#include "atlas/Simd.h"

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

			float4 origX;
			float4 origY;
			float4 origZ;

			float4 invDirX;
			float4 invDirY;
			float4 invDirZ;

			bool4 signX;
			bool4 signY;
			bool4 signZ;

			Ray() = default;
			Ray(const glm::vec3 &o, const glm::vec3 &d)
				: origin(o), dir(glm::normalize(d))
				, origX(o.x), origY(o.y), origZ(o.z)
				, invDirX(1.f / dir.x), invDirY(1.f / dir.y), invDirZ(1.f / dir.z)
			{
				invDir = 1.f / dir;
				sign[0] = (invDir.x < 0);
				sign[1] = (invDir.y < 0);
				sign[2] = (invDir.z < 0);

				signX = invDirX < float4(0.f);
				signY = invDirY < float4(0.f);
				signZ = invDirZ < float4(0.f);
			}
		};
	}
}