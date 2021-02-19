#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "../atlas/includes/Atlas/Material.h"
#include "Shape.h"

#include "../atlas/includes/Atlas/Rendering/HitRecord.h"
#include "../atlas/includes/Atlas/Rendering/Ray.h"

class Sphere : public Shape
{
public:
	Sphere(glm::vec3 c, float r, const atlas::Material *m)
		: center(c), radius(r), material(m)
	{
		bound = Bound(center - glm::vec3(r),
			center + glm::vec3(r));

		invRadius = 1 / radius;
	}

	void hit(const Cone &ray, const float min, const float max) const override
	{
		float tmax = 0;
		for (uint32_t i = 0; i < ray.nbrRays; i++)
		{
			HitRecord tmp;
			if (hit(ray.r[i], min, ray.h[i].t, tmp))
				ray.h[i] = tmp;

			float dist = glm::dot(ray.bound.dir, (ray.r[i].origin + ray.r[i].dir * ray.h[i].t) - ray.bound.origin);
			tmax = std::max(tmax, dist);
		}
		ray.bound.tmax = tmax;
	}

	bool hit(const atlas::rendering::Ray &ray, const float min, const float max, HitRecord &record) const override
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
				record.hit = true;
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
				record.hit = true;
				return (true);
			}
		}
		record.hit = false;
		return (false);
	}

	~Sphere() override {};

private:
	glm::vec3 center;
	float radius;
	float invRadius;

	const atlas::Material *material;
};