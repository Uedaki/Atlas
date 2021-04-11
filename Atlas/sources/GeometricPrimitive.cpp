#include "atlas/primitives/GeometricPrimitive.h"

using namespace atlas;

Bounds3f GeometricPrimitive::worldBound() const
{
	return (shape->worldBound());
}

bool GeometricPrimitive::intersect(const Ray &r, SurfaceInteraction &intersection) const
{
	Float tHit;
	if (!shape->intersect(r, tHit, intersection))
		return (false);
	r.tmax = tHit;
	intersection.primitive = this;

	if (mediumInterface.isMediumTransition())
		intersection.mediumInterface = mediumInterface;
	else
		intersection.mediumInterface = MediumInterface(r.medium);
	return (true);
}

bool GeometricPrimitive::intersectP(const Ray &r) const
{
	return (shape->intersectP(r));
}

void GeometricPrimitive::intersect(const Payload &p, std::vector<SurfaceInteraction> &intersections, std::vector<Float> &tmax) const
{
	Float coneMax = 0;
	for (uint32_t i = p.first; i < p.first + p.size; i++)
	{
		Float tHit = 0;
		Ray r(p.batch->origins[i], p.batch->directions[i], tmax[i]);
		if (shape->intersect(r, tHit, intersections[i]))
		{
			tmax[i] = tHit;
			intersections[i].primitive = this;
		}
		coneMax = std::max(coneMax, tHit);
	}
	p.cone.tmax = coneMax;
}

//void GeometricPrimitive::intersect(const ConeRay &cone, SurfaceInteraction &intersections) const
//{
//	DCHECK(cone.rays && intersections);
//
//	Float tmax = 0;
//	for (uint32_t i = 0; i < cone.nbrRays; i++)
//	{
//		if (shape->intersect(cone.rays[i], intersections[i]))
//		{
//			cone.rays[i].tmax = intersections[i].t;
//		}
//
//		Float dist = dot(cone.dir, cone.rays[i].dir * cone.rays[i].tmax);
//		if (dist > tmax)
//			tmax = dist;
//	}
//	cone.tmax = tmax;
//}
//
//void GeometricPrimitive::intersectP(const ConeRay &cone) const
//{
//	DCHECK(cone.rays);
//
//	Float tmax = 0;
//	for (uint32_t i = 0; i < cone.nbrRays; i++)
//	{
//		if (shape->intersectP(cone.rays[i]))
//		{
//			cone.rays[i].tmax = 0;
//		}
//
//		Float dist = dot(cone.dir, (cone.rays[i].origin + cone.rays[i].dir * cone.rays[i].tmax) - cone.origin);
//		if (dist > tmax)
//			tmax = dist;
//	}
//	cone.tmax = tmax;
//}
//
//#ifdef _USE_SIMD
//S4Bool GeometricPrimitive::intersect(const S4Ray &ray, S4SurfaceInteraction &intersection) const
//{
//	S4Bool mask = shape->intersect(ray, intersection);
//	if (any(mask))
//	{
//		ray.tmax = select(ray.tmax, intersection.t, mask);
//	}
//	return (mask);
//}
//
//S4Bool GeometricPrimitive::intersectP(const S4Ray &ray) const
//{
//	return (shape->intersectP(ray));
//}
//
//void GeometricPrimitive::intersect(const S4ConeRay &cone, S4SurfaceInteraction *intersections) const
//{
//	DCHECK(cone.rays && intersections);
//
//	S4Float tmax = S4Float(0.f);
//	for (uint32_t i = 0; i < cone.maxNbrRays; i++)
//	{
//		S4Bool mask = shape->intersect(cone.rays[i], intersections[i]);
//		if (any(mask))
//		{
//			cone.rays[i].tmax = select(cone.rays[i].tmax, intersections[i].t, mask);
//		}
//
//		S4Float dist = dot(cone.dir, (cone.rays[i].origin + cone.rays[i].dir * cone.rays[i].tmax) - cone.origin);
//		S4Bool distMsk = dist > tmax;
//		if (any(distMsk))
//			tmax = select(tmax, dist, distMsk);
//	}
//	cone.tmax = tmax;
//}
//
//void GeometricPrimitive::intersectP(const S4ConeRay &cone) const
//{
//	DCHECK(cone.rays);
//
//	S4Float tmax = S4Float(0.f);
//	for (uint32_t i = 0; i < cone.maxNbrRays; i++)
//	{
//		S4Bool mask = shape->intersectP(cone.rays[i]);
//		if (any(mask))
//		{
//			cone.rays[i].tmax = select(cone.rays[i].tmax, S4Float(0.f), mask);
//		}
//
//		S4Float dist = dot(cone.dir, (cone.rays[i].origin + cone.rays[i].dir * cone.rays[i].tmax) - cone.origin);
//		S4Bool distMsk = dist > tmax;
//		if (any(distMsk))
//			tmax = select(tmax, dist, distMsk);
//	}
//	cone.tmax = tmax;
//}
//#endif

const AreaLight *GeometricPrimitive::getAreaLight() const
{
	return areaLight.get();
}

#if defined(SHADING)
const sh::Material *GeometricPrimitive::getMaterial() const
{
	return material.get();
}
#else
const Material *GeometricPrimitive::getMaterial() const
{
	return material.get();
}
#endif

void GeometricPrimitive::computeScatteringFunctions(
	SurfaceInteraction &isect, TransportMode mode,
	bool allowMultipleLobes) const
{
#if !defined(SHADING)
	if (material)
		material->computeScatteringFunctions(isect, mode,
			allowMultipleLobes);
#endif
}