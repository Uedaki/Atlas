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
	bound = newBound;

	if (src.size() <= 2)
	{
		elements[0] = src[0];
		elements[1] = src[1];
		return;
	}

	int splitAxis = bound.largestAxis();
	std::sort(src.begin(), src.end(), [splitAxis](const Hitable *a, const Hitable *b)
		{
			return (a->getBound().getMin()[splitAxis] < b->getBound().getMin()[splitAxis]);
		});

	size_t n = 1;
	size_t f = src.size() - 2;

	Bound nearBound;
	std::vector<const Hitable *> nearElements;
	nearElements.emplace_back(src.front());
	nearBound = src.front()->getBound();

	Bound farBound;
	std::vector<const Hitable *> farElements;
	farElements.emplace_back(src.back());
	farBound = src.back()->getBound();

	while (n <= f)
	{
		Bound v1 = nearBound + src[n]->getBound();
		Bound v2 = farBound + src[n]->getBound();

		if (n != f && v1.length(splitAxis) <= v2.length(splitAxis))
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
		if (v2.length(splitAxis) <= v1.length(splitAxis))
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

	if (nearElements.size() != 1)
	{
		pool.emplace_back();
		elements[0] = &pool.back();
		pool.back().feed(nearElements, nearBound, pool);
	}
	else
		elements[0] = nearElements.front();

	if (farElements.size() != 1)
	{
		pool.emplace_back();
		elements[1] = &pool.back();
		pool.back().feed(farElements, farBound, pool);
	}
	else
		elements[1] = farElements.front();
}

bool atlas::rendering::Acceleration::hit(const Ray &ray, const float min, const float max, HitRecord &record) const
{
	HitRecord tmpRecord;
	float closest = max;
	bool hasHitAnything = false;

	if (elements[0]->getBound().intersect(ray, min, max) && elements[0]->hit(ray, 0.01f, closest, tmpRecord))
	{
		record = tmpRecord;
		closest = record.t;
		hasHitAnything = true;
	}

	if (elements[1]->getBound().intersect(ray, min, closest) && elements[1]->hit(ray, 0.01f, closest, tmpRecord))
	{
		record = tmpRecord;
		hasHitAnything = true;
	}
	return (hasHitAnything);
}

bool4 atlas::rendering::Acceleration::simdHit(SimdRay &ray, float4 min, float4 max, SimdHitRecord &record) const
{
	SimdHitRecord tmpRecord;
	record.t = max;

	bool4 res(0.f);

	bool4 msk = elements[0]->getBound().simdIntersect(ray, min, max);
	if (any(msk))
	{
		float4 zero(0.f);

		SimdRay newRay;

		newRay.origX = select(zero, ray.origX, msk);
		newRay.origY = select(zero, ray.origY, msk);
		newRay.origZ = select(zero, ray.origZ, msk);

		newRay.dirX = select(zero, ray.dirX, msk);
		newRay.dirY = select(zero, ray.dirY, msk);
		newRay.dirZ = select(zero, ray.dirZ, msk);

		newRay.invDirX = select(zero, ray.invDirX, msk);
		newRay.invDirY = select(zero, ray.invDirY, msk);
		newRay.invDirZ = select(zero, ray.invDirZ, msk);

		newRay.signX = ray.signX;
		newRay.signY = ray.signY;
		newRay.signZ = ray.signZ;

		msk = elements[0]->simdHit(newRay, min, max, tmpRecord);
		if (any(msk))
		{
			record.t = select(record.t, tmpRecord.t, msk);

			record.pX = select(record.pX, tmpRecord.pX, msk);
			record.pY = select(record.pX, tmpRecord.pY, msk);
			record.pZ = select(record.pX, tmpRecord.pZ, msk);

			record.normalX = select(record.normalX, tmpRecord.normalX, msk);
			record.normalY = select(record.normalX, tmpRecord.normalY, msk);
			record.normalZ = select(record.normalX, tmpRecord.normalZ, msk);
			
			record.texel.r = select(record.texel.r, tmpRecord.texel.r, msk);
			record.texel.g = select(record.texel.g, tmpRecord.texel.g, msk);
			record.texel.b = select(record.texel.b, tmpRecord.texel.b, msk);

			res = msk;
		}
	}
	
	msk = elements[1]->getBound().simdIntersect(ray, min, record.t);
	if (any(msk))
	{
		float4 zero(0.f);

		SimdRay newRay;

		newRay.origX = select(zero, ray.origX, msk);
		newRay.origY = select(zero, ray.origY, msk);
		newRay.origZ = select(zero, ray.origZ, msk);

		newRay.dirX = select(zero, ray.dirX, msk);
		newRay.dirY = select(zero, ray.dirY, msk);
		newRay.dirZ = select(zero, ray.dirZ, msk);

		newRay.invDirX = select(zero, ray.invDirX, msk);
		newRay.invDirY = select(zero, ray.invDirY, msk);
		newRay.invDirZ = select(zero, ray.invDirZ, msk);

		newRay.signX = ray.signX;
		newRay.signY = ray.signY;
		newRay.signZ = ray.signZ;

		msk = elements[1]->simdHit(newRay, min, record.t, tmpRecord);
		if (any(msk))
		{
			record.t = select(record.t, tmpRecord.t, msk);

			record.pX = select(record.pX, tmpRecord.pX, msk);
			record.pY = select(record.pX, tmpRecord.pY, msk);
			record.pZ = select(record.pX, tmpRecord.pZ, msk);

			record.normalX = select(record.pX, tmpRecord.normalX, msk);
			record.normalY = select(record.pX, tmpRecord.normalY, msk);
			record.normalZ = select(record.pX, tmpRecord.normalZ, msk);

			record.texel.r = select(record.texel.r, tmpRecord.texel.r, msk);
			record.texel.g = select(record.texel.g, tmpRecord.texel.g, msk);
			record.texel.b = select(record.texel.b, tmpRecord.texel.b, msk);

			res = res | msk;
		}
	}

	return (res);
}