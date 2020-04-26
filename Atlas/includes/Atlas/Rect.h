#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Material.h"
#include "Shape.h"

#include "Atlas/Rendering/HitRecord.h"
#include "Atlas/Rendering/Ray.h"

namespace atlas
{
	class RectXY : public Shape
	{
	public:
		RectXY(float x0, float x1, float y0, float y1, float k, const Material *m)
			: x0(x0), x1(x1), y0(y0), y1(y1), k(k), material(m)
		{
			bound = Bound(glm::vec3(x0, y0, k - 0.001), glm::vec3(x1, y1, k + 0.001));
		}

		bool hit(const rendering::Ray &ray, const float min, const float max, rendering::HitRecord &record) const override
		{
			float t = (k - ray.origin.z) / ray.dir.z;
			if (t < min || t > max)
				return (false);

			float x = ray.origin.x + t * ray.dir.x;
			float y = ray.origin.y + t * ray.dir.y;
			if (x < x0 || x > x1 || y < y0 || y > y1)
				return (false);

			record.uv.x = (x - x0) / (x1 - x0);
			record.uv.y = (y - y0) / (y1 - y0);
			record.t = t;
			record.material = material;
			record.p = ray.origin + ray.dir * t;
			record.normal = -1.f * glm::normalize(glm::vec3(0, 0, 1) * glm::dot(glm::vec3(0, 0, 1), ray.dir));
			return (true);
		}

		~RectXY() override {};

	private:
		float x0;
		float x1;
		float y0;
		float y1;
		float k;
		const Material *material;
	};

	class RectXZ : public Shape
	{
	public:
		RectXZ(float x0, float x1, float z0, float z1, float k, const Material *m)
			: x0(x0), x1(x1), z0(z0), z1(z1), k(k), material(m)
		{
			bound = Bound(glm::vec3(x0, k - 0.001, z0), glm::vec3(x1, k + 0.001, z1));
		}

		bool hit(const rendering::Ray &ray, const float min, const float max, rendering::HitRecord &record) const override
		{
			float t = (k - ray.origin.y) / ray.dir.y;
			if (t < min || t > max)
				return (false);

			float x = ray.origin.x + t * ray.dir.x;
			float z = ray.origin.z + t * ray.dir.z;
			if (x < x0 || x > x1 || z < z0 || z > z1)
				return (false);

			record.uv.x = (x - x0) / (x1 - x0);
			record.uv.y = (z - z0) / (z1 - z0);
			record.t = t;
			record.material = material;
			record.p = ray.origin + ray.dir * t;
			record.normal = -1.f * glm::normalize(glm::vec3(0, 1, 0) * glm::dot(glm::vec3(0, 1, 0), ray.dir));
			return (true);
		}

		~RectXZ() override {};

	private:
		float x0;
		float x1;
		float z0;
		float z1;
		float k;
		const Material *material;
	};

	class RectYZ : public Shape
	{
	public:
		RectYZ(float y0, float y1, float z0, float z1, float k, const Material *m)
			: y0(y0), y1(y1), z0(z0), z1(z1), k(k), material(m)
		{
			bound = Bound(glm::vec3(k - 0.001, y0, z0), glm::vec3(k + 0.001, y1, z1));
		}

		bool hit(const rendering::Ray &ray, const float min, const float max, rendering::HitRecord &record) const override
		{
			float t = (k - ray.origin.x) / ray.dir.x;
			if (t < min || t > max)
				return (false);

			float y = ray.origin.y + t * ray.dir.y;
			float z = ray.origin.z + t * ray.dir.z;
			if (y < y0 || y > y1 || z < z0 || z > z1)
				return (false);

			record.uv.x = (y - y0) / (y1 - y0);
			record.uv.y = (z - z0) / (z1 - z0);
			record.t = t;
			record.material = material;
			record.p = ray.origin + ray.dir * t;
			record.normal = -1.f * glm::normalize(glm::vec3(1, 0, 0) * glm::dot(glm::vec3(1, 0, 0), ray.dir));
			return (true);
		}

		~RectYZ() override {};

	private:
		float y0;
		float y1;
		float z0;
		float z1;
		float k;
		const Material *material;
	};
}