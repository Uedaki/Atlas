#pragma once

#include "Bound.h"
#include "Cone.h"

//#include "Simd.h"

namespace atlas
{
	namespace rendering {
		struct HitRecord;
	};
	namespace rendering {
		struct Ray;
	};
};

class Hitable
{
public:
	virtual bool hit(const atlas::rendering::Ray &ray, const float min, const float max, HitRecord &record) const = 0;
	virtual void hit(const Cone &ray, const float min, const float max) const = 0;

	inline const Bound &getBound() const {
		return (bound);
	}
	inline void setBound(const Bound &value) {
		bound = value;
	}
protected:
	Bound bound;
};