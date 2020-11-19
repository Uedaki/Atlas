#include "pch.h"
#include "Bound.h"

#include <algorithm>

#include "../Atlas/includes/Atlas/rendering/Ray.h"

Bound::Bound(const glm::vec3 &min, const glm::vec3 &max)
	: min(min), max(max)
{}

void Bound::operator=(const Bound &bound)
{
	max = bound.max;
	min = bound.min;
}

void Bound::operator=(const atlas::Bound &bound)
{
	max = bound.getMax();
	min = bound.getMin();
}

Bound Bound::operator+(const Bound &bound)
{
	//assert(max.x > min.x || max.y > min.y || max.z > min.z); // Bound has not been initialized.
	return (Bound(glm::vec3(std::min(min.x, bound.min.x),
		std::min(min.y, bound.min.y),
		std::min(min.z, bound.min.z)),
		glm::vec3(std::max(max.x, bound.max.x),
			std::max(max.y, bound.max.y),
			std::max(max.z, bound.max.z))));
}

Bound Bound::operator+(const atlas::Bound &bound)
{
	//assert(max.x > min.x || max.y > min.y || max.z > min.z); // Bound has not been initialized.
	return (Bound(glm::vec3(std::min(min.x, bound.getMin().x),
		std::min(min.y, bound.getMin().y),
		std::min(min.z, bound.getMin().z)),
		glm::vec3(std::max(max.x, bound.getMax().x),
			std::max(max.y, bound.getMax().y),
			std::max(max.z, bound.getMax().z))));
}

void Bound::operator+=(const Bound &bound)
{
	assert(max.x > min.x || max.y > min.y || max.z > min.z); // Bound has not been initialized.
	max = glm::vec3(std::max(max.x, bound.max.x),
		std::max(max.y, bound.max.y),
		std::max(max.z, bound.max.z));
	min = glm::vec3(std::min(min.x, bound.min.x),
		std::min(min.y, bound.min.y),
		std::min(min.z, bound.min.z));
}

void Bound::operator+=(const atlas::Bound &bound)
{
	assert(max.x > min.x || max.y > min.y || max.z > min.z); // Bound has not been initialized.
	max = glm::vec3(std::max(max.x, bound.getMax().x),
		std::max(max.y, bound.getMax().y),
		std::max(max.z, bound.getMax().z));
	min = glm::vec3(std::min(min.x, bound.getMin().x),
		std::min(min.y, bound.getMin().y),
		std::min(min.z, bound.getMin().z));
}

int Bound::largestAxis() const
{
	glm::vec3 axis(max.x - min.x,
		max.y - min.y,
		max.z - min.z);
	if (axis.x < axis.y)
	{
		if (axis.y < axis.z)
			return (2);
		else
			return (1);
	}
	else
	{
		if (axis.x < axis.z)
			return (2);
		else
			return (0);
	}
}

glm::vec3 Bound::getCenter() const
{
	return (min + (max - min) * 0.5f);
}

bool Bound::intersect(const atlas::rendering::Ray &ray, float distMin, float distMax) const
{
	float tmin;
	float tmax;
	float tymin;
	float tymax;
	float tzmin;
	float tzmax;

	glm::vec3 bounds[2] = { min, max };

	tmin = (bounds[ray.sign[0]].x - ray.origin.x) * ray.invDir.x;
	tmax = (bounds[1 - ray.sign[0]].x - ray.origin.x) * ray.invDir.x;
	tymin = (bounds[ray.sign[1]].y - ray.origin.y) * ray.invDir.y;
	tymax = (bounds[1 - ray.sign[1]].y - ray.origin.y) * ray.invDir.y;

	if (tmin > tymax || tymin > tmax)
	{
		return (false);
	}
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	tzmin = (bounds[ray.sign[2]].z - ray.origin.z) * ray.invDir.z;
	tzmax = (bounds[1 - ray.sign[2]].z - ray.origin.z) * ray.invDir.z;

	if (tmin > tzmax || tzmin > tmax)
	{
		return (false);
	}
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	if (tmin < distMax && distMin < tmax)
		return (true);
	return (false);
}



// ref: Intersection of a Box and a Cone or Cone Frustum by David Eberly, Geometric Tools, Redmond WA 98052
// https://www.geometrictools.com/Documentation/IntersectionBoxCone.pdf
bool Bound::intersectCone(const BoundingCone &cone, float distMin, float distMax) const
{
	return false;
}

bool4 Bound::simdIntersect(SimdRay &ray, float4 distMin, float4 distMax) const
{
	float4 tmin;
	float4 tmax;
	float4 tymin;
	float4 tymax;
	float4 tzmin;
	float4 tzmax;

	float4 minBoundX(min.x);
	float4 minBoundY(min.y);
	float4 minBoundZ(min.z);

	float4 maxBoundX(max.x);
	float4 maxBoundY(max.y);
	float4 maxBoundZ(max.z);

	float4 lowerBoundX = select(minBoundX, maxBoundX, ray.signX);
	float4 lowerBoundY = select(minBoundY, maxBoundY, ray.signY);
	float4 lowerBoundZ = select(minBoundZ, maxBoundZ, ray.signZ);

	float4 upperBoundX = select(maxBoundX, minBoundX, ray.signX);
	float4 upperBoundY = select(maxBoundY, minBoundY, ray.signY);
	float4 upperBoundZ = select(maxBoundZ, minBoundZ, ray.signZ);

	tmin = (lowerBoundX - ray.origX) * ray.invDirX;
	tmax = (upperBoundX - ray.origX) * ray.invDirX;
	tymin = (lowerBoundY - ray.origY) * ray.invDirY;
	tymax = (upperBoundY - ray.origY) * ray.invDirY;

	bool4 msk = (tmin > tymax) | (tymin > tmax);

	if (all(msk))
		return (bool4(0.f));

	bool4 tminMsk = tymin > tmin;
	tmin = select(tmin, tymin, tminMsk);

	bool4 tmaxMsk = tymax < tmax;
	tmax = select(tmax, tymax, tmaxMsk);

	tzmin = (lowerBoundZ - ray.origZ) * ray.invDirZ;
	tzmax = (upperBoundZ - ray.origZ) * ray.invDirZ;

	msk = (tmin > tzmax) | (tzmin > tmax);
	if (all(msk))
		return (bool4(0.f));

	tminMsk = tzmin > tmin;
	tmin = select(tmin, tzmin, tminMsk);

	tmaxMsk = tzmax < tmax;
	tmax = select(tmax, tzmax, tmaxMsk);

	return ((tmin < distMax) & (distMin < tmax));
}