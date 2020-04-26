#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Material.h"
#include "Shape.h"

#include "Atlas/Rendering/HitRecord.h"
#include "Atlas/Rendering/Ray.h"

namespace atlas
{
	class Sphere : public Shape
	{
	public:
		Sphere(glm::vec3 c, float r, const Material *m)
			: center(c), radius(r), material(m)
		{
			bound = Bound(center - glm::vec3(r),
				center + glm::vec3(r));

			invRadius = 1 / radius;
		}

		bool hit(const rendering::Ray &ray, const float min, const float max, rendering::HitRecord &record) const override
		{
			record.material = material;

			glm::vec3 oc = ray.origin - center;
			float a = glm::dot(ray.dir, ray.dir);
			float b = glm::dot(oc, ray.dir);
			float c = glm::dot(oc, oc) - radius * radius;
			float discriminant = b * b - a * c;
			if (discriminant > 0)
			{
				float invA = 1 / a;
				float temp1 = (-b - sqrt(discriminant)) * invA;
				float temp2 = (-b + sqrt(discriminant)) * invA;
				if (min < temp1 && temp1 < max)
				{
					record.t = temp1;
					record.p = ray.origin + ray.dir * temp1;
					record.normal = (record.p - center) * invRadius;

					glm::vec3 n = glm::normalize(record.p - center);
					float phi = atan2(n.z, n.x);
					float theta = asin(n.y);
					record.uv.x = 1 - (phi + glm::pi<float>()) / (2.f * glm::pi<float>());
					record.uv.y = (theta + glm::pi<float>() / 2.f) / glm::pi<float>();

					return (true);
				}
				else if (min < temp2 && temp2 < max)
				{
					record.t = temp2;
					record.p = ray.origin + ray.dir * temp2;
					record.normal = (record.p - center) * invRadius;
					
					glm::vec3 n = glm::normalize(record.p - center);
					float phi = atan2(n.z, n.x);
					float theta = asin(n.y);
					record.uv.x = 1 - (phi + glm::pi<float>()) / (2.f * glm::pi<float>());
					record.uv.y = (theta + glm::pi<float>() / 2.f) / glm::pi<float>();

					return (true);
				}
			}
			return (false);
		}

		bool4 simdHit(SimdRay &ray, float4 min, float4 max, SimdHitRecord &record) const override
		{
			float4 sqRadius(radius * radius);

			float4 centerX(center.x);
			float4 centerY(center.y);
			float4 centerZ(center.z);

			float4 ocX = ray.origX - centerX;
			float4 ocY = ray.origY - centerY;
			float4 ocZ = ray.origZ - centerZ;

			float4 a = ray.dirX * ray.dirX + ray.dirY * ray.dirY + ray.dirZ * ray.dirZ;
			float4 b = ocX * ray.dirX + ocY * ray.dirY + ocZ * ray.dirZ;
			float4 c = ocX * ocX + ocY * ocY + ocZ * ocZ - sqRadius;

			float4 discriminant = b * b - a * c;

			bool4 discriminantMsk = discriminant > float4(0.f);
			if (any(discriminantMsk))
			{
				float4 invA = float4(1.f) / a;
				float4 t0 = (-b - sqrtf(discriminant)) * invA;
				float4 t1 = (-b + sqrtf(discriminant)) * invA;

				bool4 msk0 = discriminantMsk & (t0 > min) & (t0 < max);
				bool4 msk1 = discriminantMsk & (t1 > min) & (t1 < max);

				float4 t = select(t1, t0, msk0);
				bool4 msk = msk0 | msk1;

				if (any(msk))
				{
					record.t = select(max, t, msk);

					float4 posX = ray.origX + ray.dirX * t;
					float4 posY = ray.origY + ray.dirY * t;
					float4 posZ = ray.origZ + ray.dirZ * t;
					record.pX = select(record.pX, posX, msk);
					record.pY = select(record.pY, posY, msk);
					record.pZ = select(record.pZ, posZ, msk);


					float4 r(1.0f);

					record.texel.r = select(record.texel.r, r, msk);
					record.texel.g = select(record.texel.g, r, msk);
					record.texel.b = select(record.texel.b, r, msk);

					return (msk);
				}
			}
			return (bool4(0.f));
		}

		~Sphere() override {};

	private:
		glm::vec3 center;
		float radius;
		float invRadius;

		const Material *material;
	};
}