#include "atlas/core/Shape.h"

using namespace atlas;

Shape::Shape(const Shape::Info &info)
	: objectToWorld(*info.objectToWorld)
	, worldToObject(*info.worldToObject)
	, reverseOrientation(info.reverseOrientation)
	, transformSwapsHandedness(false)
{}

Shape::Shape(const Transform &objectToWorld, const Transform &worldToObject, bool reverseOrientation)
	: objectToWorld(objectToWorld)
	, worldToObject(worldToObject)
	, reverseOrientation(reverseOrientation)
	, transformSwapsHandedness(false)
{}

Bounds3f Shape::worldBound() const
{
	return (objectToWorld(objectBound()));
}

Interaction Shape::sample(const Interaction &ref, const Point2f &u, Float &pdf) const
{
	Interaction intr = sample(u, pdf);
	Vec3f wi = intr.p - ref.p;
	if (wi.lengthSquared() == 0)
	{
		pdf = 0;
	}
	else
	{
		wi = normalize(wi);
		pdf *= distanceSquared(ref.p, intr.p) / std::abs(dot(intr.n, -wi));
		if (std::isinf(pdf))
			pdf = 0;
	}
	return (intr);
}

Float Shape::pdf(const Interaction &ref, const Vec3f &wi) const
{
	Ray ray = ref.spawnRay(wi);
	Float tHit;
	SurfaceInteraction isectLight;
	if (!intersect(ray, tHit, isectLight, false))
		return (0);

	Float pdf = distanceSquared(ref.p, isectLight.p) / (std::abs(dot(isectLight.n, -wi)) * area());
	if (std::isinf(pdf))
		pdf = 0.f;
	return (pdf);
}