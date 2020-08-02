#include "pch.h"
#include "Rendering/Acceleration.h"

#include <algorithm>

#include "Rendering/HitRecord.h"
#include "Rendering/Ray.h"

atlas::rendering::Acceleration::Acceleration(std::vector<const Hitable *> &src, const Bound &newBound, std::vector<Acceleration> &pool)
{
	feed(src, newBound, pool);
}

void atlas::rendering::Acceleration::feed(std::vector<const Hitable *> &src, const Bound &newBound, std::vector<Acceleration> &pool)
{
#ifdef SIMD
	std::array<Bound, 4> bounds;
#endif
	bound = newBound;

	if (src.size() <= 4)
	{
		if (src.size() > 0)
		{
			elements[0] = src[0];
			bounds[0] = src[0]->getBound();
		}
		if (src.size() > 1)
		{
			elements[1] = src[1];
			bounds[1] = src[1]->getBound();
		}
		if (src.size() > 2)
		{
			elements[2] = src[2];
			bounds[2] = src[2]->getBound();
		}
		if (src.size() > 3)
		{
			elements[3] = src[3];
			bounds[3] = src[3]->getBound();
		}
	}
	else
	{
		int splitAxis = bound.largestAxis();
		Bound nearBound;
		std::vector<const Hitable *> nearElements;
		Bound farBound;
		std::vector<const Hitable *> farElements;
		split(src, nearElements, nearBound, farElements, farBound, splitAxis);

		splitAxis = nearBound.largestAxis();
		Bound nearBound0;
		std::vector<const Hitable *> nearElements0;
		Bound nearBound1;
		std::vector<const Hitable *> nearElements1;
		split(nearElements, nearElements0, nearBound0, nearElements1, nearBound1, splitAxis);

		splitAxis = farBound.largestAxis();
		Bound farBound0;
		std::vector<const Hitable *> farElements0;
		Bound farBound1;
		std::vector<const Hitable *> farElements1;
		split(farElements, farElements0, farBound0, farElements1, farBound1, splitAxis);

		if (nearElements0.size() > 1)
		{
			pool.emplace_back();
			elements[0] = &pool.back();
			bounds[0] = nearBound0;
			pool.back().feed(nearElements0, nearBound0, pool);
		}
		else if (nearElements0.size() == 1)
		{
			elements[0] = nearElements0.front();
			bounds[0] = nearElements0.front()->getBound();
		}

		if (nearElements1.size() > 1)
		{
			pool.emplace_back();
			elements[1] = &pool.back();
			bounds[1] = nearBound1;
			pool.back().feed(nearElements1, nearBound1, pool);
		}
		else if (nearElements1.size() > 1)
		{
			elements[1] = nearElements1.front();
			bounds[1] = nearElements1.front()->getBound();
		}

		if (farElements0.size() > 1)
		{
			pool.emplace_back();
			elements[2] = &pool.back();
			bounds[2] = farBound0;
			pool.back().feed(farElements0, farBound0, pool);
		}
		else if (farElements0.size() == 1)
		{
			elements[2] = farElements0.front();
			bounds[2] = farElements0.front()->getBound();
		}
		if (farElements1.size() > 1)
		{
			pool.emplace_back();
			elements[3] = &pool.back();
			bounds[3] = farBound1;
			pool.back().feed(farElements1, farBound1, pool);
		}
		else if (farElements1.size() == 1)
		{
			elements[3] = farElements1.front();
			bounds[3] = farElements1.front()->getBound();
		}
	}

#ifdef SIMD
	minBoundX = float4(bounds[0].getMin().x, bounds[1].getMin().x, bounds[2].getMin().x, bounds[3].getMin().x);
	minBoundY = float4(bounds[0].getMin().y, bounds[1].getMin().y, bounds[2].getMin().y, bounds[3].getMin().y);
	minBoundZ = float4(bounds[0].getMin().z, bounds[1].getMin().z, bounds[2].getMin().z, bounds[3].getMin().z);

	maxBoundX = float4(bounds[0].getMax().x, bounds[1].getMax().x, bounds[2].getMax().x, bounds[3].getMax().x);
	maxBoundY = float4(bounds[0].getMax().y, bounds[1].getMax().y, bounds[2].getMax().y, bounds[3].getMax().y);
	maxBoundZ = float4(bounds[0].getMax().z, bounds[1].getMax().z, bounds[2].getMax().z, bounds[3].getMax().z);
#endif
}

void atlas::rendering::Acceleration::split(std::vector<const Hitable *> &src,
	std::vector<const Hitable *> &nearElements, Bound &nearBound,
	std::vector<const Hitable *> &farElements, Bound &farBound,
	int axis)
{
	std::sort(src.begin(), src.end(), [axis](const Hitable *a, const Hitable *b)
		{
			return (a->getBound().getMin()[axis] < b->getBound().getMin()[axis]);
		});

	size_t n = 1;
	size_t f = src.size() - 2;
	if (src.size() < 2)
		f = 0;

	if (!src.empty())
	{
		nearElements.emplace_back(src.front());
		nearBound = src.front()->getBound();
	}

	if (src.size() > 1)
	{
		farElements.emplace_back(src.back());
		farBound = src.back()->getBound();
	}

	while (n <= f)
	{
		Bound v1 = nearBound + src[n]->getBound();
		Bound v2 = farBound + src[n]->getBound();

		if (n != f && v1.length(axis) <= v2.length(axis))
		{
			nearElements.emplace_back(src[n]);
			nearBound = v1;
			n += 1;
		}
		else
		{
			for (size_t i = n; i <= f; i++)
			{
				farElements.emplace_back(src[i]);
				farBound += src[i]->getBound();
			}
			break;
		}

		v1 = nearBound + src[f]->getBound();
		v2 = farBound + src[f]->getBound();
		if (v2.length(axis) <= v1.length(axis))
		{
			farElements.emplace_back(src[f]);
			farBound = v2;
			f -= 1;
		}
		else
		{
			for (size_t i = n; i <= f; i++)
			{
				nearElements.emplace_back(src[i]);
				nearBound += src[i]->getBound();
			}
			break;
		}
	}
}

bool atlas::rendering::Acceleration::hit(const Ray &ray, const float min, const float max, HitRecord &record) const
{
#ifndef SIMD

	HitRecord tmpRecord;
	float closest = max;
	bool hasHitAnything = false;

	if (bounds[0].intersect(ray, min, closest) && elements[0]->hit(ray, 0.01f, closest, tmpRecord))
	{
		record = tmpRecord;
		closest = record.t;
		hasHitAnything = true;
	}

	if (bounds[1].intersect(ray, min, closest) && elements[1]->hit(ray, 0.01f, closest, tmpRecord))
	{
		record = tmpRecord;
		closest = record.t;
		hasHitAnything = true;
	}

	if (bounds[2].intersect(ray, min, closest) && elements[2]->hit(ray, 0.01f, closest, tmpRecord))
	{
		record = tmpRecord;
		closest = record.t;
		hasHitAnything = true;
	}

	if (bounds[3].intersect(ray, min, closest) && elements[3]->hit(ray, 0.01f, closest, tmpRecord))
	{
		record = tmpRecord;
		closest = record.t;
		hasHitAnything = true;
	}
	return (hasHitAnything);
#else
	HitRecord tmpRecord;
	float closest = max;
	bool hasHitAnything = false;

	bool4 msk = simdIntersect(ray, min, max);

	float mskf[4];
	_mm_store_ps(mskf, msk.m);
	if (isnan(mskf[0]) && elements[0]->hit(ray, 0.01f, closest, tmpRecord))
	{
		record = tmpRecord;
		closest = record.t;
		hasHitAnything = true;
	}

	if (isnan(mskf[1]) && elements[1]->hit(ray, 0.01f, closest, tmpRecord))
	{
		record = tmpRecord;
		closest = record.t;
		hasHitAnything = true;
	}

	if (isnan(mskf[2]) && elements[2]->hit(ray, 0.01f, closest, tmpRecord))
	{
		record = tmpRecord;
		closest = record.t;
		hasHitAnything = true;
	}

	if (isnan(mskf[3]) && elements[3]->hit(ray, 0.01f, closest, tmpRecord))
	{
		record = tmpRecord;
		closest = record.t;
		hasHitAnything = true;
	}

	return(hasHitAnything);
#endif
}

#ifdef SIMD
bool4 atlas::rendering::Acceleration::simdIntersect(const Ray &ray, const float min, const float max) const
{
	float4 tmin;
	float4 tmax;
	float4 tymin;
	float4 tymax;
	float4 tzmin;
	float4 tzmax;
	
	float4 distMin(min);
	float4 distMax(max);

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

	bool4 msk0 = (tmin <= tymax) & (tymin <= tmax);
	if (!any(msk0))
		return (bool4(0.f));

	bool4 tminMsk = tymin > tmin;
	tmin = select(tmin, tymin, tminMsk);

	bool4 tmaxMsk = tymax < tmax;
	tmax = select(tmax, tymax, tmaxMsk);

	tzmin = (lowerBoundZ - ray.origZ) * ray.invDirZ;
	tzmax = (upperBoundZ - ray.origZ) * ray.invDirZ;

	bool4 msk1 = (tmin <= tzmax) & (tzmin <= tmax);
	if (!any(msk1))
		return (bool4(0.f));

	tminMsk = tzmin > tmin;
	tmin = select(tmin, tzmin, tminMsk);

	tmaxMsk = tzmax < tmax;
	tmax = select(tmax, tzmax, tmaxMsk);

	bool4 msk2 = (tmin <= distMax) & (distMin <= tmax);
	return (msk0 & msk1 & msk2);
}
#endif