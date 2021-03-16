#include "atlas/shapes/Sphere.h"

#include "atlas/core/Bounds.h"
#include "atlas/core/EFloat.h"
#include "atlas/core/Geometry.h"
#include "atlas/core/Sampling.h"
#include "atlas/core/Vectors.h"

using namespace atlas;

Bounds3f Sphere::objectBound() const
{
	return (Bounds3f(Point3f(-radius, -radius, zMin),
		Point3f(radius, radius, zMax)));
}

bool Sphere::intersect(const Ray &r, Float &tHit, SurfaceInteraction &intersection, bool testAlphaTexture) const
{
    Float phi;
    Point3f pHit;

    Vec3f oErr, dErr;
    Ray ray = worldToObject(r, oErr, dErr);

    EFloat ox(ray.origin.x, oErr.x), oy(ray.origin.y, oErr.y), oz(ray.origin.z, oErr.z);
    EFloat dx(ray.dir.x, dErr.x), dy(ray.dir.y, dErr.y), dz(ray.dir.z, dErr.z);
    EFloat a = dx * dx + dy * dy + dz * dz;
    EFloat b = 2 * (dx * ox + dy * oy + dz * oz);
    EFloat c = ox * ox + oy * oy + oz * oz - EFloat(radius) * EFloat(radius);

    EFloat t0, t1;
    if (!quadratic(a, b, c, &t0, &t1))
        return false;

    if (t0.upperBound() > ray.tmax || t1.lowerBound() <= 0)
        return false;
    EFloat tShapeHit = t0;
    if (tShapeHit.lowerBound() <= 0)
    {
        tShapeHit = t1;
        if (tShapeHit.upperBound() > ray.tmax)
            return false;
    }

    pHit = ray((Float)tShapeHit);

    pHit *= radius / distance(pHit, Point3f(0, 0, 0));
    if (pHit.x == 0 && pHit.y == 0) pHit.x = 1e-5f * radius;
    phi = std::atan2(pHit.y, pHit.x);
    if (phi < 0) phi += 2 * PI;

    if ((zMin > -radius && pHit.z < zMin) || (zMax < radius && pHit.z > zMax) || phi > phiMax)
    {
        if (tShapeHit == t1)
            return false;
        if (t1.upperBound() > ray.tmax)
            return false;
        tShapeHit = t1;
        pHit = ray((Float)tShapeHit);

        pHit *= radius / distance(pHit, Point3f(0, 0, 0));
        if (pHit.x == 0 && pHit.y == 0)
            pHit.x = 1e-5f * radius;
        phi = std::atan2(pHit.y, pHit.x);
        if (phi < 0)
            phi += 2 * PI;
        if ((zMin > -radius && pHit.z < zMin) ||
            (zMax < radius && pHit.z > zMax) || phi > phiMax)
            return false;
    }

    Float u = phi / phiMax;
    Float theta = std::acos(clamp(pHit.z / radius, -1, 1));
    Float v = (theta - thetaMin) / (thetaMax - thetaMin);

    Float zRadius = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
    Float invZRadius = 1 / zRadius;
    Float cosPhi = pHit.x * invZRadius;
    Float sinPhi = pHit.y * invZRadius;
    Vec3f dpdu(-phiMax * pHit.y, phiMax * pHit.x, 0);
    Vec3f dpdv = (thetaMax - thetaMin) *
        Vec3f(pHit.z * cosPhi, pHit.z * sinPhi, -radius * std::sin(theta));

    Vec3f d2Pduu = -phiMax * phiMax * Vec3f(pHit.x, pHit.y, 0);
    Vec3f d2Pduv =
        (thetaMax - thetaMin) * pHit.z * phiMax * Vec3f(-sinPhi, cosPhi, 0.);
    Vec3f d2Pdvv = -(thetaMax - thetaMin) * (thetaMax - thetaMin) *
        Vec3f(pHit.x, pHit.y, pHit.z);

    Float E = dot(dpdu, dpdu);
    Float F = dot(dpdu, dpdv);
    Float G = dot(dpdv, dpdv);
    Vec3f N = normalize(cross(dpdu, dpdv));
    Float e = dot(N, d2Pduu);
    Float f = dot(N, d2Pduv);
    Float g = dot(N, d2Pdvv);

    Float invEGF2 = 1 / (E * G - F * F);
    Normal dndu = Normal((f * F - e * G) * invEGF2 * dpdu +
        (e * F - f * E) * invEGF2 * dpdv);
    Normal dndv = Normal((g * F - f * G) * invEGF2 * dpdu +
        (f * F - g * E) * invEGF2 * dpdv);

    Vec3f pError = gamma(5) * abs((Vec3f)pHit);

    intersection = objectToWorld(SurfaceInteraction(pHit, pError, Point2f(u, v),
        -ray.dir, dpdu, dpdv, dndu, dndv,
        ray.time, this));

    // Update _tHit_ for quadric intersection
    tHit = (Float)tShapeHit;
    return true;
}

bool Sphere::intersectP(const Ray &r, bool testAlphaTexture) const // TODO
{
    Float phi;
    Point3f pHit;

    Vec3f oErr;
    Vec3f dErr;
    Ray ray = worldToObject(r, oErr, dErr);

    EFloat ox(ray.origin.x, oErr.x);
    EFloat oy(ray.origin.y, oErr.y);
    EFloat oz(ray.origin.z, oErr.z);
    EFloat dx(ray.dir.x, dErr.x);
    EFloat dy(ray.dir.y, dErr.y);
    EFloat dz(ray.dir.z, dErr.z);
    EFloat a = dx * dx + dy * dy + dz * dz;
    EFloat b = 2 * (dx * ox + dy * oy + dz * oz);
    EFloat c = ox * ox + oy * oy + oz * oz - EFloat(radius) * EFloat(radius);

    EFloat t0;
    EFloat t1;
    if (!quadratic(a, b, c, &t0, &t1))
        return false;

    if (t0.upperBound() > ray.tmax || t1.lowerBound() <= 0)
        return false;
    EFloat tShapeHit = t0;
    if (tShapeHit.lowerBound() <= 0)
    {
        tShapeHit = t1;
        if (tShapeHit.upperBound() > ray.tmax) return false;
    }

    pHit = ray((Float)tShapeHit);

    pHit *= radius / distance(pHit, Point3f(0, 0, 0));
    if (pHit.x == 0 && pHit.y == 0)
        pHit.x = 1e-5f * radius;
    phi = std::atan2(pHit.y, pHit.x);
    if (phi < 0)
        phi += 2 * PI;

    if ((zMin > -radius && pHit.z < zMin) || (zMax < radius && pHit.z > zMax) || phi > phiMax)
    {
        if (tShapeHit == t1)
            return false;
        if (t1.upperBound() > ray.tmax)
            return false;
        tShapeHit = t1;
        pHit = ray((Float)tShapeHit);

        pHit *= radius / distance(pHit, Point3f(0, 0, 0));
        if (pHit.x == 0 && pHit.y == 0)
            pHit.x = 1e-5f * radius;
        phi = std::atan2(pHit.y, pHit.x);
        if (phi < 0)
            phi += 2 * PI;
        if ((zMin > -radius && pHit.z < zMin) || (zMax < radius && pHit.z > zMax) || phi > phiMax)
            return false;
    }
    return true;
}

//S4Bool Sphere::intersect(const S4Ray &ray, S4SurfaceInteraction &intersection) const
//{
//	S4Bool hit(0.f);
//	S4Float sRadius(radius);
//
//	S4Vec3 oc = ray.origin - S4Point3(center);
//	S4Float a = dot(ray.dir, ray.dir);
//	S4Float b = dot(oc, ray.dir);
//	S4Float c = dot(oc, oc) - sRadius * sRadius;
//	S4Float discriminant = b * b - a * c;
//	
//	S4Bool isDiscPositive = discriminant > S4Float(0.f);
//	if (any(isDiscPositive))
//	{
//		S4Float invA = S4Float(1) / a;
//		S4Float temp1 = (-b - sqrtf(discriminant)) * invA;
//		S4Float temp2 = (-b + sqrtf(discriminant)) * invA;
//		
//		S4Bool temp1Mask = isDiscPositive & (S4Float(0.f) < temp1) & (temp1 < ray.tmax);
//		if (any(temp1Mask))
//		{
//			intersection.t = select(intersection.t, temp1, temp1Mask);
//			intersection.p = select(intersection.p, ray.origin + ray.dir * temp1, temp1Mask);
//			intersection.normal = select(intersection.normal, (intersection.p - S4Point3(center)) * S4Float(1 / radius), temp1Mask);
//
//			hit = hit | temp1Mask;
//		}
//		
//		S4Bool temp2Mask = isDiscPositive & (S4Float(0.f) < temp2) & (temp2 < ray.tmax);
//		if (any(temp2Mask))
//		{
//			intersection.t = select(intersection.t, temp2, temp2Mask);
//			intersection.p = select(intersection.p, ray.origin + ray.dir * temp2, temp2Mask);
//			intersection.normal = select(intersection.normal, (intersection.p - S4Point3(center)) * S4Float(1 / radius), temp2Mask);
//
//			hit = hit | temp1Mask;
//		}
//	}
//	return (hit);
//}
//
//S4Bool shape::Sphere::intersectP(const S4Ray &ray) const
//{
//	S4Bool hit(0.f);
//	S4Float sRadius(radius);
//
//	S4Vec3 oc = ray.origin - S4Point3(center);
//	S4Float a = dot(ray.dir, ray.dir);
//	S4Float b = dot(oc, ray.dir);
//	S4Float c = dot(oc, oc) - sRadius * sRadius;
//	S4Float discriminant = b * b - a * c;
//
//	S4Bool isDiscPositive = discriminant > S4Float(0.f);
//	if (any(isDiscPositive))
//	{
//		S4Float invA = S4Float(1) / a;
//		S4Float temp1 = (-b - sqrtf(discriminant)) * invA;
//		S4Float temp2 = (-b + sqrtf(discriminant)) * invA;
//
//		S4Bool temp1Mask = isDiscPositive & (((S4Float(0.f) < temp1) & (temp1 < ray.tmax)) | ((S4Float(0.f) < temp2) & (temp2 < ray.tmax)));
//	}
//	return (hit);
//}

Float Sphere::area() const
{
	return phiMax * radius * (zMax - zMin);
}

Interaction Sphere::sample(const Point2f &u, Float &pdf) const
{
	Point3f pObj = Point3f(0) + radius * uniformSampleSphere(u);

	Interaction it;
	it.n = normalize(objectToWorld(Normal(pObj.x, pObj.y, pObj.z)));
	if (reverseOrientation)
		it.n *= -1;
	pObj *= radius / distance(pObj, Point3f(0));
	Vec3f pObjError = gamma(5) * abs((Vec3f)pObj);
	it.p = objectToWorld(pObj, pObjError, it.pError);
	pdf = 1 / area();
	return (it);
}

Interaction Sphere::sample(const Interaction &ref, const Point2f &u, Float &pdf) const
{
    Point3f pCenter = objectToWorld(Point3f(0, 0, 0));

    Point3f pOrigin = offsetRayOrigin(ref.p, ref.pError, ref.n, pCenter - ref.p);
    if (distanceSquared(pOrigin, pCenter) <= radius * radius)
	{
        Interaction intr = sample(u, pdf);
        Vec3f wi = intr.p - ref.p;
        if (wi.lengthSquared() == 0)
            pdf = 0;
        else
		{
            wi = normalize(wi);
            pdf *= distanceSquared(ref.p, intr.p) / std::abs(dot(intr.n, -wi));
        }
        if (std::isinf(pdf))
			pdf = 0.f;
        return intr;
    }

    Float dc = distance(ref.p, pCenter);
    Float invDc = 1 / dc;
    Vec3f wc = (pCenter - ref.p) * invDc;
    Vec3f wcX, wcY;
    coordinateSystem(wc, &wcX, &wcY);

    Float sinThetaMax = radius * invDc;
    Float sinThetaMax2 = sinThetaMax * sinThetaMax;
    Float invSinThetaMax = 1 / sinThetaMax;
    Float cosThetaMax = std::sqrt(std::max((Float)0.f, 1 - sinThetaMax2));

    Float cosTheta = (cosThetaMax - 1) * u[0] + 1;
    Float sinTheta2 = 1 - cosTheta * cosTheta;

    if (sinThetaMax2 < 0.00068523f /* sin^2(1.5 deg) */)
	{
        sinTheta2 = sinThetaMax2 * u[0];
        cosTheta = std::sqrt(1 - sinTheta2);
    }

	Float cosAlpha = sinTheta2 * invSinThetaMax +
        cosTheta * std::sqrt(std::max((Float)0.f, 1.f - sinTheta2 * invSinThetaMax * invSinThetaMax));
    Float sinAlpha = std::sqrt(std::max((Float)0.f, 1.f - cosAlpha * cosAlpha));
    Float phi = u[1] * 2 * PI;

    Vec3f nWorld = sphericalDirection(sinAlpha, cosAlpha, phi, -wcX, -wcY, -wc);
    Point3f pWorld = pCenter + radius * Point3f(nWorld.x, nWorld.y, nWorld.z);

    Interaction it;
    it.p = pWorld;
    it.pError = gamma(5) * abs((Vec3f)pWorld);
    it.n = Normal(nWorld);
    if (reverseOrientation)
		it.n *= -1;

    pdf = 1 / (2 * PI * (1 - cosThetaMax));

    return it;
}

Float Sphere::pdf(const Interaction &ref, const Vec3f &wi) const
{
	Point3f pCenter = objectToWorld(Point3f(0, 0, 0));
	Point3f pOrigin = offsetRayOrigin(ref.p, ref.pError, ref.n, pCenter - ref.p);
	if (distanceSquared(pOrigin, pCenter) <= radius * radius)
		return Shape::pdf(ref, wi);

	Float sinThetaMax2 = radius * radius / distanceSquared(ref.p, pCenter);
	Float cosThetaMax = std::sqrt(std::max((Float)0, 1 - sinThetaMax2));
	return (uniformConePdf(cosThetaMax));
}

std::shared_ptr<Shape> Sphere::createShape(const Sphere::Info &info)
{
	return (std::make_shared<Sphere>(info));
}