#pragma once

#include "Hitable.h"

class Shape : public Hitable
{
public:
	virtual ~Shape() = default;

	bool hit(const atlas::rendering::Ray &ray, const float min, const float max, HitRecord &record) const override = 0;
	void hit(const Cone &ray, const float min, const float max) const override = 0;

private:
};