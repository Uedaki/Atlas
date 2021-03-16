#include "atlas/core/Interaction.h"

#include "atlas/core/Primitive.h"
#include "atlas/core/Shape.h"

using namespace atlas;

SurfaceInteraction::SurfaceInteraction(
    const Point3f &p, const Vec3f &pError, const Point2f &uv,
    const Vec3f &wo, const Vec3f &dpdu, const Vec3f &dpdv,
    const Normal &dndu, const Normal &dndv, Float time, const Shape *shape,
    int faceIndex)
    : Interaction(p, Normal(normalize(cross(dpdu, dpdv))), pError, wo, time,
        nullptr),
    uv(uv),
    dpdu(dpdu),
    dpdv(dpdv),
    dndu(dndu),
    dndv(dndv),
    shape(shape),
    faceIndex(faceIndex)
{

    shading.n = n;
    shading.dpdu = dpdu;
    shading.dpdv = dpdv;
    shading.dndu = dndu;
    shading.dndv = dndv;

    if (shape && (shape->reverseOrientation ^ shape->transformSwapsHandedness))
    {
        n *= -1;
        shading.n *= -1;
    }
}

void SurfaceInteraction::setShadingGeometry(const Vec3f &dpdus,
    const Vec3f &dpdvs,
    const Normal &dndus,
    const Normal &dndvs,
    bool orientationIsAuthoritative)
{
    shading.n = normalize((Normal)cross(dpdus, dpdvs));
    if (orientationIsAuthoritative)
        n = faceForward(n, shading.n);
    else
        shading.n = faceForward(shading.n, n);

    shading.dpdu = dpdus;
    shading.dpdv = dpdvs;
    shading.dndu = dndus;
    shading.dndv = dndvs;
}

void SurfaceInteraction::computeScatteringFunctions(const RayDifferential &ray,
    bool allowMultipleLobes,
    TransportMode mode)
{
    computeDifferentials(ray);
    primitive->computeScatteringFunctions(*this, mode, allowMultipleLobes);
}

void SurfaceInteraction::computeDifferentials(const RayDifferential &ray) const
{
    if (ray.hasDifferentials)
    {
        Float d = dot(n, Vec3f(p.x, p.y, p.z));
        Float tx = -(dot(n, Vec3f(ray.rxOrigin)) - d) / dot(n, ray.rxDirection);
        if (std::isinf(tx) || std::isnan(tx))
            goto fail;
        Point3f px = ray.rxOrigin + tx * ray.rxDirection;
        Float ty = -(dot(n, Vec3f(ray.ryOrigin)) - d) / dot(n, ray.ryDirection);
        if (std::isinf(ty) || std::isnan(ty))
            goto fail;
        Point3f py = ray.ryOrigin + ty * ray.ryDirection;
        dpdx = px - p;
        dpdy = py - p;

        int dim[2];
        if (std::abs(n.x) > std::abs(n.y) && std::abs(n.x) > std::abs(n.z))
        {
            dim[0] = 1;
            dim[1] = 2;
        }
        else if (std::abs(n.y) > std::abs(n.z))
        {
            dim[0] = 0;
            dim[1] = 2;
        }
        else
        {
            dim[0] = 0;
            dim[1] = 1;
        }

        Float A[2][2] = { {dpdu[dim[0]], dpdv[dim[0]]},
                         {dpdu[dim[1]], dpdv[dim[1]]} };
        Float Bx[2] = { px[dim[0]] - p[dim[0]], px[dim[1]] - p[dim[1]] };
        Float By[2] = { py[dim[0]] - p[dim[0]], py[dim[1]] - p[dim[1]] };
        if (!solveLinearSystem2x2(A, Bx, dudx, dvdx))
            dudx = dvdx = 0;
        if (!solveLinearSystem2x2(A, By, dudy, dvdy))
            dudy = dvdy = 0;
    }
    else
    {
    fail: // TODO DELETE LABEL/GOTO
        dudx = dvdx = 0;
        dudy = dvdy = 0;
        dpdx = dpdy = Vec3f(0, 0, 0);
    }
}

Spectrum SurfaceInteraction::le(const Vec3f &w) const
{
    const AreaLight *area = primitive->getAreaLight();
   // return (area ? area->L(*this, w) : Spectrum(0.f));
    return (Spectrum(0.f));
}
