#include "atlas/shapes/Triangle.h"

atlas::Triangle::Triangle(const Transform &objectToWorld, const Transform &worldToObject, bool reverseOrientation, const std::shared_ptr<TriangleMesh> &mesh, uint32_t triNumber)
    : Shape(objectToWorld, worldToObject, reverseOrientation)
    , mesh(mesh)
{
    v = &mesh->vertexIndices[3 * triNumber];
}

atlas::Bounds3f atlas::Triangle::objectBound() const
{
    const Point3f &p0 = mesh->p[v[0]];
    const Point3f &p1 = mesh->p[v[1]];
    const Point3f &p2 = mesh->p[v[2]];
    return expand(Bounds3f((worldToObject)(p0), (worldToObject)(p1)),
        (worldToObject)(p2));
}

atlas::Bounds3f atlas::Triangle::worldBound() const
{
    const Point3f &p0 = mesh->p[v[0]];
    const Point3f &p1 = mesh->p[v[1]];
    const Point3f &p2 = mesh->p[v[2]];
    return expand(Bounds3f(p0, p1), p2);
}

bool atlas::Triangle::intersect(const Ray &ray, Float &tHit, SurfaceInteraction &isect, bool testAlphaTexture) const
{
    // Get triangle vertices in _p0_, _p1_, and _p2_
    const Point3f &p0 = mesh->p[v[0]];
    const Point3f &p1 = mesh->p[v[1]];
    const Point3f &p2 = mesh->p[v[2]];

    // Perform ray--triangle intersection test

    // Transform triangle vertices to ray coordinate space

    // Translate vertices based on ray origin
    Point3f p0t = p0 - Vec3f(ray.origin);
    Point3f p1t = p1 - Vec3f(ray.origin);
    Point3f p2t = p2 - Vec3f(ray.origin);

    // Permute components of triangle vertices and ray direction
    int kz = abs(ray.dir).maxDimension();
    int kx = kz + 1;
    if (kx == 3) kx = 0;
    int ky = kx + 1;
    if (ky == 3) ky = 0;
    Vec3f d = permute(ray.dir, kx, ky, kz);
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
    if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
        return false;
    Float det = e0 + e1 + e2;
    if (det == 0) return false;

    // Compute scaled hit distance to triangle and test against ray $t$ range
    p0t.z *= Sz;
    p1t.z *= Sz;
    p2t.z *= Sz;
    Float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
    if (det < 0 && (tScaled >= 0 || tScaled < ray.tmax * det))
        return false;
    else if (det > 0 && (tScaled <= 0 || tScaled > ray.tmax * det))
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
    GetUVs(uv);

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
    isect = SurfaceInteraction(pHit, pError, uvHit, -ray.dir, dpdu, dpdv,
        Normal(0, 0, 0), Normal(0, 0, 0), ray.time,
        this, faceIndex);

    // Override surface normal in _isect_ for triangle
    isect.n = isect.shading.n = Normal(normalize(cross(dp02, dp12)));
    if (reverseOrientation ^ transformSwapsHandedness)
        isect.n = isect.shading.n = -isect.n;

    if (mesh->n || mesh->s) {
        // Initialize _Triangle_ shading geometry

        // Compute shading normal _ns_ for triangle
        Normal ns;
        if (mesh->n) {
            ns = (b0 * mesh->n[v[0]] + b1 * mesh->n[v[1]] + b2 * mesh->n[v[2]]);
            if (ns.lengthSquared() > 0)
                ns = normalize(ns);
            else
                ns = isect.n;
        }
        else
            ns = isect.n;

        // Compute shading tangent _ss_ for triangle
        Vec3f ss;
        if (mesh->s) {
            ss = (b0 * mesh->s[v[0]] + b1 * mesh->s[v[1]] + b2 * mesh->s[v[2]]);
            if (ss.lengthSquared() > 0)
                ss = normalize(ss);
            else
                ss = normalize(isect.dpdu);
        }
        else
            ss = normalize(isect.dpdu);

        // Compute shading bitangent _ts_ for triangle and adjust _ss_
        Vec3f ts = cross(ss, ns);
        if (ts.lengthSquared() > 0.f) {
            ts = normalize(ts);
            ss = cross(ts, ns);
        }
        else
            coordinateSystem((Vec3f)ns, &ss, &ts);

        // Compute $\dndu$ and $\dndv$ for triangle shading geometry
        Normal dndu, dndv;
        if (mesh->n)
        {
            // Compute deltas for triangle partial derivatives of normal
            Vec2f duv02 = uv[0] - uv[2];
            Vec2f duv12 = uv[1] - uv[2];
            Normal dn1 = mesh->n[v[0]] - mesh->n[v[2]];
            Normal dn2 = mesh->n[v[1]] - mesh->n[v[2]];
            Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
            bool degenerateUV = std::abs(determinant) < 1e-8;
            if (degenerateUV) {
                // We can still compute dndu and dndv, with respect to the
                // same arbitrary coordinate system we use to compute dpdu
                // and dpdv when this happens. It's important to do this
                // (rather than giving up) so that ray differentials for
                // rays reflected from triangles with degenerate
                // parameterizations are still reasonable.
                Vec3f dn = cross(Vec3f(mesh->n[v[2]] - mesh->n[v[0]]),
                    Vec3f(mesh->n[v[1]] - mesh->n[v[0]]));
                if (dn.lengthSquared() == 0)
                    dndu = dndv = Normal(0, 0, 0);
                else {
                    Vec3f dnu, dnv;
                    coordinateSystem(dn, &dnu, &dnv);
                    dndu = Normal(dnu);
                    dndv = Normal(dnv);
                }
            }
            else {
                Float invDet = 1 / determinant;
                dndu = (duv12[1] * dn1 - duv02[1] * dn2) * invDet;
                dndv = (-duv12[0] * dn1 + duv02[0] * dn2) * invDet;
            }
        }
        else
            dndu = dndv = Normal(0, 0, 0);
        if (reverseOrientation) ts = -ts;
        isect.setShadingGeometry(ss, ts, dndu, dndv, true);
    }

    tHit = t;
    return true;
}

bool atlas::Triangle::intersectP(const Ray &ray, bool testAlphaTexture) const
{
    // Get triangle vertices in _p0_, _p1_, and _p2_
    const Point3f &p0 = mesh->p[v[0]];
    const Point3f &p1 = mesh->p[v[1]];
    const Point3f &p2 = mesh->p[v[2]];

    // Perform ray--triangle intersection test

    // Transform triangle vertices to ray coordinate space

    // Translate vertices based on ray origin
    Point3f p0t = p0 - Vec3f(ray.origin);
    Point3f p1t = p1 - Vec3f(ray.origin);
    Point3f p2t = p2 - Vec3f(ray.origin);

    // Permute components of triangle vertices and ray direction
    int kz = abs(ray.dir).maxDimension();
    int kx = kz + 1;
    if (kx == 3) kx = 0;
    int ky = kx + 1;
    if (ky == 3) ky = 0;
    Vec3f d = permute(ray.dir, kx, ky, kz);
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
    if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
        return false;
    Float det = e0 + e1 + e2;
    if (det == 0) return false;

    // Compute scaled hit distance to triangle and test against ray $t$ range
    p0t.z *= Sz;
    p1t.z *= Sz;
    p2t.z *= Sz;
    Float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
    if (det < 0 && (tScaled >= 0 || tScaled < ray.tmax * det))
        return false;
    else if (det > 0 && (tScaled <= 0 || tScaled > ray.tmax * det))
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
    return true;
}

Float atlas::Triangle::area() const
{
    // Get triangle vertices in _p0_, _p1_, and _p2_
    const Point3f &p0 = mesh->p[v[0]];
    const Point3f &p1 = mesh->p[v[1]];
    const Point3f &p2 = mesh->p[v[2]];
    return 0.5 * cross(p1 - p0, p2 - p0).length();
}

atlas::Interaction atlas::Triangle::sample(const Point2f &u, Float &pdf) const
{
    Point2f b = uniformSampleTriangle(u);
    // Get triangle vertices in _p0_, _p1_, and _p2_
    const Point3f &p0 = mesh->p[v[0]];
    const Point3f &p1 = mesh->p[v[1]];
    const Point3f &p2 = mesh->p[v[2]];
    Interaction it;
    it.p = b[0] * p0 + b[1] * p1 + (1 - b[0] - b[1]) * p2;
    // Compute surface normal for sampled point on triangle
    it.n = normalize(Normal(cross(p1 - p0, p2 - p0)));
    // Ensure correct orientation of the geometric normal; follow the same
    // approach as was used in Triangle::Intersect().
    if (mesh->n) {
        Normal ns(b[0] * mesh->n[v[0]] + b[1] * mesh->n[v[1]] +
            (1 - b[0] - b[1]) * mesh->n[v[2]]);
        it.n = faceForward(it.n, ns);
    }
    else if (reverseOrientation ^ transformSwapsHandedness)
        it.n *= -1;

    // Compute error bounds for sampled point on triangle
    Point3f pAbsSum =
        abs(b[0] * p0) + abs(b[1] * p1) + abs((1 - b[0] - b[1]) * p2);
    it.pError = gamma(6) * Vec3f(pAbsSum.x, pAbsSum.y, pAbsSum.z);
    pdf = 1 / area();
    return it;
}

std::vector<std::shared_ptr<atlas::Shape>> atlas::createTriangleMesh(const Transform &objectToWorld, const Transform &worldToObject, bool reverseOrientation, uint32_t nTriangles, const uint32_t *vertexIndices, uint32_t nVertices, const Point3f *p
    , const Vec3f *s, const Normal *n, const Point2f *uv)
{
    std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(objectToWorld, nTriangles, vertexIndices, nVertices, p, s, n, uv, 0);
    std::vector<std::shared_ptr<Shape>> tris;
    for (uint32_t i = 0; i < nTriangles; i++)
    {
        tris.push_back(std::make_shared<Triangle>(objectToWorld, worldToObject, false, mesh, i));
    }
    return (tris);
}
