#pragma once

#include <array>
#include <vector>

#include "Hitable.h"

#include "../Atlas/includes/atlas/Simd.h"

#include "Cone.h"
#include "Bound.h"

# ifdef ACHERON_EXPORTS
#   define ACH  __declspec( dllexport )
# else
#   define ACH __declspec( dllimport )
# endif

//#define SIMD

class Acceleration : public Hitable
{
public:
	Acceleration() = default;
	ACH Acceleration(std::vector<const Hitable *> &source, const Bound &bound, std::vector<Acceleration> &pool);
	ACH void feed(std::vector<const Hitable *> &source, const Bound &bound, std::vector<Acceleration> &pool);

	ACH bool hit(const atlas::rendering::Ray &ray, const float min, const float max, HitRecord &record) const override;
	ACH void hit(const Cone &ray, const float min, const float max) const override;

private:
	std::array<const Hitable *, 4> elements = { nullptr };

#ifndef SIMD
	std::array<Bound, 4> bounds;
#endif

	ACH void split(std::vector<const Hitable *> &src,
		std::vector<const Hitable *> &nearElements, Bound &nearBound,
		std::vector<const Hitable *> &farElements, Bound &farBound,
		int axis);

#ifdef SIMD
	bool4 simdIntersect(const atlas::rendering::Ray &ray, const float min, const float max) const;

	float4 minBoundX;
	float4 minBoundY;
	float4 minBoundZ;

	float4 maxBoundX;
	float4 maxBoundY;
	float4 maxBoundZ;
#endif
};