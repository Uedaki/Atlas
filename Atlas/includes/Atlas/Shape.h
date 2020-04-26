#pragma once

#include "Atlas/Hitable.h"

namespace atlas
{
	class Shape : public Hitable
	{
	public:
		virtual ~Shape() = default;

		bool hit(const rendering::Ray &ray, const float min, const float max, rendering::HitRecord &record) const override = 0;

	private:
	};
}