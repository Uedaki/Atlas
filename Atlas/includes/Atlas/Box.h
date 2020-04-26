#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Material.h"
#include "Shape.h"

#include "Atlas/Rendering/HitRecord.h"
#include "Atlas/Rendering/Ray.h"
#include "Atlas/Rect.h"

namespace atlas
{
	class Box : public Shape
	{
	public:
		Box(const glm::vec3 &p0, const glm::vec3 &p1, const Material *m)
			: p0(p0), p1(p1), material(m)
			, up(p0.x, p1.x, p0.y, p1.y, p1.z, m)
			, down(p0.x, p1.x, p0.y, p1.y, p0.z, m)
			, front(p0.x, p1.x, p0.z, p1.z, p1.y, m)
			, back(p0.x, p1.x, p0.z, p1.z, p0.y, m)
			, left(p0.y, p1.y, p0.z, p1.z, p1.x, m)
			, right(p0.y, p1.y, p0.z, p1.z, p0.x, m)
		{
			bound = Bound(p0, p1);
			
		}

		bool hit(const rendering::Ray &ray, const float min, const float max, rendering::HitRecord &record) const override
		{
			bool ret = false;
			float m = max;

			rendering::HitRecord upRecord;
			if (up.hit(ray, min, m, upRecord))
			{
				record = upRecord;
				m = record.t;
				ret = true;
			}

			rendering::HitRecord downRecord;
			if (down.hit(ray, min, m, downRecord))
			{
				record = downRecord;
				m = record.t;
				ret = true;
			}

			rendering::HitRecord frontRecord;
			if (front.hit(ray, min, m, frontRecord))
			{
				record = frontRecord;
				m = record.t;
				ret = true;
			}

			rendering::HitRecord backRecord;
			if (back.hit(ray, min, m, backRecord))
			{
				record = backRecord;
				m = record.t;
				ret = true;
			}

			rendering::HitRecord leftRecord;
			if (front.hit(ray, min, m, leftRecord))
			{
				record = leftRecord;
				m = record.t;
				ret = true;
			}

			rendering::HitRecord rightRecord;
			if (front.hit(ray, min, m, rightRecord))
			{
				record = rightRecord;
				m = record.t;
				ret = true;
			}

			return (ret);
		}

		~Box() override {};

	private:
		glm::vec3 p0;
		glm::vec3 p1;

		RectXY up;
		RectXY down;
		RectXZ front;
		RectXZ back;
		RectYZ left;
		RectYZ right;

		const Material *material;
		
	};
}