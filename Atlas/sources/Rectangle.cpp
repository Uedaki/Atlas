#include "Atlas/Shapes/Rectangle.h"

#include "atlas/core/Geometry.h"

atlas::Bounds3f atlas::Rectangle::objectBound() const
{
	return (Bounds3f(Point3f(-1, -1, -0.1), Point3f(1, 1, 0.1)));
}

bool atlas::Rectangle::intersect(const Ray &r, Float &tHit, SurfaceInteraction &intersection, bool testAlphaTexture) const
{
	Vec3f oErr, dErr;
	Ray ray = worldToObject(r, oErr, dErr);


	Float t = (0 - ray.origin.z) / ray.dir.z;
	Point3f pHit = ray(t);
	if (-1 < pHit.y && pHit.y < 1
		&& -1 < pHit.x && pHit.x < 1)
	{
		Float u = pHit.x * 0.5 + 1;
		Float v = pHit.y * 0.5 + 1;

		Vec3f dpdu(1, 0, 0);
		Vec3f dpdv(0, 1, 0);

		Vec3f dnu, dnv;
		coordinateSystem(Vec3f(0, 0, 1), &dnu, &dnv);
		Normal dndu = Normal(dnu);
		Normal dndv = Normal(dnv);

		Vec3f pError = gamma(5) * abs((Vec3f)pHit);
		intersection = objectToWorld(SurfaceInteraction(pHit, pError, Point2f(u, v),
			-ray.dir, dpdu, dpdv, dndu, dndv,
			ray.time, this));

		return (true);
	}
	return (false);
}

bool atlas::Rectangle::intersectP(const Ray &ray, bool testAlphaTexture) const
{
	return (false);
}

Float atlas::Rectangle::area() const
{
	return (1 * 1);
}

atlas::Interaction atlas::Rectangle::sample(const Point2f &u, Float &pdf) const
{
	return (Interaction());
}

atlas::Interaction atlas::Rectangle::sample(const Interaction &ref, const Point2f &u, Float &pdf) const
{
	return (Interaction());
}

Float atlas::Rectangle::pdf(const Interaction &ref, const Vec3f &wi) const
{
	return (0);
}