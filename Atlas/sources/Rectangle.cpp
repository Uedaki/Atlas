#include "Atlas/Shapes/Rectangle.h"

#include "atlas/core/Geometry.h"
#include "atlas/core/EFloat.h"

atlas::Bounds3f atlas::Rectangle::objectBound() const
{
	return (Bounds3f(Point3f(-1, -1, -0.01f), Point3f(1, 1, 0.01f)));
}

atlas::Bounds3f atlas::Rectangle::worldBound() const
{
    return expand(Bounds3f(p0, p1), p2);
}

bool atlas::Rectangle::intersect(const Ray &r, Float &tHit, SurfaceInteraction &intersection, bool testAlphaTexture) const
{
    // Perform ray--triangle intersection test

    // Transform triangle vertices to ray coordinate space

    // Translate vertices based on ray origin
    Point3f p0t = p0 - Vec3f(r.origin);
    Point3f p1t = p1 - Vec3f(r.origin);
    Point3f p2t = p2 - Vec3f(r.origin);

    // Permute components of triangle vertices and ray direction
    int kz = abs(r.dir).maxDimension();
    int kx = kz + 1;
    if (kx == 3) kx = 0;
    int ky = kx + 1;
    if (ky == 3) ky = 0;
    Vec3f d = permute(r.dir, kx, ky, kz);
    p0t = permute(p0t, kx, ky, kz);
    p1t = permute(p1t, kx, ky, kz);
    p2t = permute(p2t, kx, ky, kz);

    // Apply shear transformation to translated vertex positions
    Float Sx = -d.x / d.z;
    Float Sy = -d.y / d.z;
    Float Sz = 1.f / d.z;
    p0t.x += Sx * p0t.z;
    p0t.y += Sy * p0t.z;
    p1t.x += Sx * p1t.z;
    p1t.y += Sy * p1t.z;
    p2t.x += Sx * p2t.z;
    p2t.y += Sy * p2t.z;

    // Compute edge function coefficients _e0_, _e1_, and _e2_
    Float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
    Float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
    Float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

    // Fall back to double precision test at triangle edges
    if (sizeof(Float) == sizeof(float) &&
        (e0 == 0.0f || e1 == 0.0f || e2 == 0.0f)) {
        double p2txp1ty = (double)p2t.x * (double)p1t.y;
        double p2typ1tx = (double)p2t.y * (double)p1t.x;
        e0 = (float)(p2typ1tx - p2txp1ty);
        double p0txp2ty = (double)p0t.x * (double)p2t.y;
        double p0typ2tx = (double)p0t.y * (double)p2t.x;
        e1 = (float)(p0typ2tx - p0txp2ty);
        double p1txp0ty = (double)p1t.x * (double)p0t.y;
        double p1typ0tx = (double)p1t.y * (double)p0t.x;
        e2 = (float)(p1typ0tx - p1txp0ty);
    }

    // Perform triangle edge and determinant tests
    //if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
    //    return false;
    Float det = e0 + e1 + e2;
    if (det == 0) return false;

    // Compute scaled hit distance to triangle and test against ray $t$ range
    p0t.z *= Sz;
    p1t.z *= Sz;
    p2t.z *= Sz;
    Float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
    if (det < 0 && (tScaled >= 0 || tScaled < r.tmax * det))
        return false;
    else if (det > 0 && (tScaled <= 0 || tScaled > r.tmax * det))
        return false;

    // Compute barycentric coordinates and $t$ value for triangle intersection
    Float invDet = 1 / det;
    Float b0 = e0 * invDet;
    Float b1 = e1 * invDet;
    Float b2 = e2 * invDet;
    Float t = tScaled * invDet;

    // Ensure that computed triangle $t$ is conservatively greater than zero

    // Compute $\delta_z$ term for triangle $t$ error bounds
    Float maxZt = abs(Vec3f(p0t.z, p1t.z, p2t.z)).maxComponent();
    Float deltaZ = gamma(3) * maxZt;

    // Compute $\delta_x$ and $\delta_y$ terms for triangle $t$ error bounds
    Float maxXt = abs(Vec3f(p0t.x, p1t.x, p2t.x)).maxComponent();
    Float maxYt = abs(Vec3f(p0t.y, p1t.y, p2t.y)).maxComponent();
    Float deltaX = gamma(5) * (maxXt + maxZt);
    Float deltaY = gamma(5) * (maxYt + maxZt);

    // Compute $\delta_e$ term for triangle $t$ error bounds
    Float deltaE =
        2 * (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);

    // Compute $\delta_t$ term for triangle $t$ error bounds and check _t_
    Float maxE = abs(Vec3f(e0, e1, e2)).maxComponent();
    Float deltaT = 3 *
        (gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE) *
        std::abs(invDet);
    if (t <= deltaT) return false;

    // Compute triangle partial derivatives
    Vec3f dpdu, dpdv;
    Point2f uv[3];
    uv[0] = 0;
    uv[1] = 0;
    uv[2] = 0;
    //GetUVs(uv);

    // Compute deltas for triangle partial derivatives
    Vec2f duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
    Vec3f dp02 = p0 - p2, dp12 = p1 - p2;
    Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
    bool degenerateUV = std::abs(determinant) < 1e-8;
    if (!degenerateUV) {
        Float invdet = 1 / determinant;
        dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
        dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
    }
    if (degenerateUV || cross(dpdu, dpdv).lengthSquared() == 0) {
        // Handle zero determinant for triangle partial derivative matrix
        Vec3f ng = cross(p2 - p0, p1 - p0);
        if (ng.lengthSquared() == 0)
            // The triangle is actually degenerate; the intersection is
            // bogus.
            return false;

        coordinateSystem(normalize(ng), &dpdu, &dpdv);
    }

    // Compute error bounds for triangle intersection
    Float xAbsSum =
        (std::abs(b0 * p0.x) + std::abs(b1 * p1.x) + std::abs(b2 * p2.x));
    Float yAbsSum =
        (std::abs(b0 * p0.y) + std::abs(b1 * p1.y) + std::abs(b2 * p2.y));
    Float zAbsSum =
        (std::abs(b0 * p0.z) + std::abs(b1 * p1.z) + std::abs(b2 * p2.z));
    Vec3f pError = gamma(7) * Vec3f(xAbsSum, yAbsSum, zAbsSum);

    // Interpolate $(u,v)$ parametric coordinates and hit point
    Point3f pHit = b0 * p0 + b1 * p1 + b2 * p2;
    Point2f uvHit = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];

    // Fill in _SurfaceInteraction_ from triangle hit
    intersection = SurfaceInteraction(pHit, pError, uvHit, -r.dir, dpdu, dpdv,
        Normal(0, 0, 0), Normal(0, 0, 0), r.time,
        this, 0);

    // Override surface normal in _isect_ for triangle
    intersection.n = intersection.shading.n = Normal(normalize(cross(dp02, dp12)));
    if (reverseOrientation ^ transformSwapsHandedness)
        intersection.n = intersection.shading.n = -intersection.n;
    tHit = t;
    return true;











	//Vec3f oErr, dErr;
	//Ray ray = worldToObject(r, oErr, dErr);

	////if (ray.dir.z >= 0)
	////	return (false);

	//double origin = ray.origin.z;
	//double dir = ray.dir.z;
	//double t = -origin / dir;
	//Point3f pHit = ray(t);
	//if (-1 < pHit.y && pHit.y < 1
	//	&& -1 < pHit.x && pHit.x < 1)
	//{
	//	Float u = pHit.x * 0.5 + 1;
	//	Float v = pHit.y * 0.5 + 1;

	//	Vec3f dpdu(1, 0, 0);
	//	Vec3f dpdv(0, 1, 0);

	//	Vec3f dnu, dnv;
	//	coordinateSystem(Vec3f(0, 0, -1), &dnu, &dnv);
	//	Normal dndu = Normal(dnu);
	//	Normal dndv = Normal(dnv);

	//	Vec3f pError = gamma(5) * abs((Vec3f)pHit);
	//	intersection = objectToWorld(SurfaceInteraction(pHit, pError, Point2f(u, v),
	//		-ray.dir, dpdu, dpdv, dndu, dndv,
	//		ray.time, this));

	//	return (true);
	//}
	//return (false);
}

bool atlas::Rectangle::intersectP(const Ray &ray, bool testAlphaTexture) const
{
	return (false);
}

Float atlas::Rectangle::area() const
{
	return (objectToWorld.getMatrix().m[0][0] * objectToWorld.getMatrix().m[1][1]);
}

atlas::Interaction atlas::Rectangle::sample(const Point2f &u, Float &pdf) const
{
	Interaction it;
	it.n = normalize(objectToWorld(Normal(0, 0, -1)));
	if (reverseOrientation)
		it.n *= -1;
	Point3f pHit(u.x * 2 - 1, u.y * 2 - 1, 0);
	Vec3f pObjError = gamma(5) * abs((Vec3f)pHit);
	it.p = objectToWorld(pHit, pObjError, it.pError);
	pdf = 1 / area();
	return (it);
}

atlas::Interaction atlas::Rectangle::sample(const Interaction &ref, const Point2f &u, Float &pdf) const
{
	Interaction it;
	it.n = normalize(objectToWorld(Normal(0, 0, -1)));
	if (reverseOrientation)
		it.n *= -1;
	Point3f pHit(u.x * 2 - 1, u.y * 2 - 1, 0);
	Vec3f pObjError = gamma(5) * abs((Vec3f)pHit);
	it.p = objectToWorld(pHit, pObjError, it.pError);
	pdf = dot(ref.p - it.p, it.n);
	pdf = pdf > 0 ? pdf : 0;
	return (it);
}

Float atlas::Rectangle::pdf(const Interaction &ref, const Vec3f &wi) const
{
	return (1);
}