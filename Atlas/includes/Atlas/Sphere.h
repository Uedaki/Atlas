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

		~Sphere() override {};

	private:
		glm::vec3 center;
		float radius;
		float invRadius;

		const Material *material;
	};
}