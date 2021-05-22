#include "TraceRays.h"

bool atlas::task::TraceRays::preExecute()
{
	tmax.resize(data.batch->size());
	for (Float &m : tmax)
		m = std::numeric_limits<Float>::max();
	return (true);
}

void atlas::task::TraceRays::execute()
{
	CHECK(data.interactions->size() != 0);

	while (true)
	{
		uint32_t index = traceRaysIndex.fetch_add(maxPackSize);
		if (index >= data.batch->size())
			break;

		uint32_t size = std::min(maxPackSize, data.batch->size() - index);
		for (uint32_t i = 0; i < size; i += maxConeSize)
		{
#if 1
			uint32_t rayCount = i + maxConeSize <= size ? maxConeSize : size - i;
			for (uint32_t j = 0; j < rayCount; j++)
			{
				Ray r(data.batch->origins[index + i + j], data.batch->directions[index + i + j], data.tmax);
				data.scene->intersect(r, (*data.interactions)[index + i + j]);
			}
#else
			Payload p;
			p.cone = packRaysInCone(i, i + maxConeSize <= size ? maxConeSize : i + maxConeSize - size);
			p.batch = data.batch;
			p.first = index;
			p.size = size;

			data.scene->intersect(p, *data.interactions, tmax);
#endif
		}
	}
}

void atlas::task::TraceRays::postExecute()
{
	tmax.clear();
}

atlas::BoundingCone atlas::task::TraceRays::packRaysInCone(uint32_t startingIndex, uint32_t size)
{
	Vec3f dir(0);
	Point3f pointOnAxe(0);
	for (uint32_t j = 0; j < size; j++)
	{
		dir += data.batch->directions[startingIndex + j];
		pointOnAxe += data.batch->origins[startingIndex + j];
	}
	dir /= (Float)size;
	pointOnAxe /= (Float)size;

	Float cosAngle = 1.f;
	Point3f ref;
	Float longestDist = std::numeric_limits<float>::min();
	for (uint32_t j = 0; j < size; j++)
	{
		Float dist;
		Float offset;
		Float dirDot = dot(dir, data.batch->directions[startingIndex + j]);
		if (dirDot < .9)
			offset = dot(dir, data.batch->origins[startingIndex + j] - pointOnAxe);
		else
			offset = (pointOnAxe - data.batch->origins[startingIndex + j]).length();

		Vec3f h = (Vec3f)pointOnAxe + dir * offset;
		Float hypLength = h.length() / dot(normalize(h - (Vec3f)data.batch->origins[startingIndex + j]), -data.batch->directions[startingIndex + j]);
		dist = dirDot * hypLength;


		if (offset + dist > longestDist)
		{
			ref = data.batch->origins[startingIndex + j];
			longestDist = offset + dist;
			cosAngle = dirDot;
		}
	}

	BoundingCone cone;
	cone.origin = pointOnAxe - dir * longestDist;
	cone.dir = dir;
	cone.dot = 0 ? cosAngle : dot(normalize(ref - cone.origin), dir);
	for (uint32_t j = 0; j < size; j++)
	{
		cone.tmin = std::min(cone.tmin, dot(cone.dir, ((Vec3f)data.batch->origins[startingIndex + j] + data.batch->directions[startingIndex + j] * (Float)0.001) - (Vec3f)cone.origin));
	}
	cone.tmax = std::numeric_limits<Float>::max();

#if 1 // test if all rays are inside the cone
	for (uint32_t j = 0; j < size; j++)
	{
		if (dot(normalize(data.batch->origins[startingIndex + j] - cone.origin), dir) < cone.dot)
		{
			float a = dot(normalize(data.batch->origins[startingIndex + j] - cone.origin), dir);
			std::cout << "cone mismatch" << std::endl;
			break;
		}

		if (dot(data.batch->directions[startingIndex + j], dir) < cone.dot)
		{
			float a = dot(data.batch->directions[startingIndex + j], dir);
			std::cout << "cone mismatch" << std::endl;
			break;
		}
	}
#endif

	return (cone);
}