#include "atlas/cameras/PerspectiveCamera.h"

#include "atlas/core/ray.h"
#include "atlas/core/random.h"
#include "atlas/core/Sampling.h"

using namespace atlas;
PerspectiveCamera::PerspectiveCamera(const Transform &CameraToWorld,
    const Bounds2f &screenWindow,
    Float shutterOpen, Float shutterClose,
    Float lensRadius, Float focalDistance,
    Float fov, Film *film,
    const Medium *medium)
    : ProjectiveCamera(CameraToWorld, perspective(fov, 1e-2f, 1000.f),
        screenWindow, shutterOpen, shutterClose, lensRadius,
        focalDistance, film, medium)
{
    dxCamera =
        (RasterToCamera(Point3f(1, 0, 0)) - RasterToCamera(Point3f(0, 0, 0)));
    dyCamera =
        (RasterToCamera(Point3f(0, 1, 0)) - RasterToCamera(Point3f(0, 0, 0)));

    Point2i res = film->resolution;
    Point3f pMin = RasterToCamera(Point3f(0, 0, 0));
    Point3f pMax = RasterToCamera(Point3f(res.x, res.y, 0));
    pMin /= pMin.z;
    pMax /= pMax.z;
    A = std::abs((pMax.x - pMin.x) * (pMax.y - pMin.y));
}

Float PerspectiveCamera::generateRay(const CameraSample &sample,
    Ray &ray) const
{
    Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
    Point3f pCamera = RasterToCamera(pFilm);
    ray = Ray(Point3f(0, 0, 0), normalize(Vec3f(pCamera)));

    if (lensRadius > 0)
    {
        Point2f pLens = lensRadius * concentricSampleDisk(sample.pLens);

        Float ft = focalDistance / ray.dir.z;
        Point3f pFocus = ray(ft);

        ray.origin = Point3f(sample.pLens.x, sample.pLens.y, 0);
        ray.dir = normalize(pFocus - ray.origin);
    }
    ray.time = lerp(sample.time, shutterOpen, shutterClose);
    ray.medium = medium;
    ray = CameraToWorld(ray);
    return 1;
}

Float PerspectiveCamera::generateRayDifferential(const CameraSample &sample,
    RayDifferential *ray) const
{
    Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
    Point3f pCamera = RasterToCamera(pFilm);
    Vec3f dir = normalize(Vec3f(pCamera.x, pCamera.y, pCamera.z));
    *ray = RayDifferential(Point3f(0, 0, 0), dir);

    if (lensRadius > 0)
    {
        Point2f pLens = lensRadius * concentricSampleDisk(sample.pLens);

        Float ft = focalDistance / ray->dir.z;
        Point3f pFocus = (*ray)(ft);

        ray->origin = Point3f(pLens.x, pLens.y, 0);
        ray->dir = normalize(pFocus - ray->origin);
    }

    if (lensRadius > 0)
    {
        Point2f pLens = lensRadius * concentricSampleDisk(sample.pLens);
        Vec3f dx = normalize(Vec3f(pCamera + dxCamera));
        Float ft = focalDistance / dx.z;
        Point3f pFocus = Point3f(0, 0, 0) + (ft * dx);
        ray->rxOrigin = Point3f(pLens.x, pLens.y, 0);
        ray->rxDirection = normalize(pFocus - ray->rxOrigin);

        Vec3f dy = normalize(Vec3f(pCamera + dyCamera));
        ft = focalDistance / dy.z;
        pFocus = Point3f(0, 0, 0) + (ft * dy);
        ray->ryOrigin = Point3f(pLens.x, pLens.y, 0);
        ray->ryDirection = normalize(pFocus - ray->ryOrigin);
    }
    else {
        ray->rxOrigin = ray->ryOrigin = ray->origin;
        ray->rxDirection = normalize(Vec3f(pCamera) + dxCamera);
        ray->ryDirection = normalize(Vec3f(pCamera) + dyCamera);
    }
    ray->time = lerp(sample.time, shutterOpen, shutterClose);
    ray->medium = medium;
    *ray = CameraToWorld(*ray);
    ray->hasDifferentials = true;
    return 1;
}

Spectrum PerspectiveCamera::we(const Ray &ray, Point2f *pRaster2) const
{
    Float cosTheta = dot(ray.dir, CameraToWorld(Vec3f(0, 0, 1)));
    if (cosTheta <= 0) return 0;

    Point3f pFocus = ray((lensRadius > 0 ? focalDistance : 1) / cosTheta);
    Point3f pRaster = RasterToCamera.inverse()(CameraToWorld.inverse()(pFocus));

    if (pRaster2)
        *pRaster2 = Point2f(pRaster.x, pRaster.y);

    Bounds2i sampleBounds = film->getSampleBounds();
    if (pRaster.x < sampleBounds.min.x || pRaster.x >= sampleBounds.max.x ||
        pRaster.y < sampleBounds.min.y || pRaster.y >= sampleBounds.max.y)
        return 0;

    Float lensArea = lensRadius != 0 ? (PI * lensRadius * lensRadius) : 1;

    Float cos2Theta = cosTheta * cosTheta;
    return Spectrum(1 / (A * lensArea * cos2Theta * cos2Theta));
}

void PerspectiveCamera::pdfWe(const Ray &ray, Float *pdfPos,
    Float *pdfDir) const
{
    Float cosTheta = dot(ray.dir, CameraToWorld(Vec3f(0, 0, 1)));
    if (cosTheta <= 0)
    {
        *pdfPos = *pdfDir = 0;
        return;
    }

    Point3f pFocus = ray((lensRadius > 0 ? focalDistance : 1) / cosTheta);
    Point3f pRaster = RasterToCamera.inverse()(CameraToWorld.inverse()(pFocus));

    Bounds2i sampleBounds = film->getSampleBounds();
    if (pRaster.x < sampleBounds.min.x || pRaster.x >= sampleBounds.max.x ||
        pRaster.y < sampleBounds.min.y || pRaster.y >= sampleBounds.max.y) {
        *pdfPos = *pdfDir = 0;
        return;
    }

    Float lensArea = lensRadius != 0 ? (PI * lensRadius * lensRadius) : 1;
    *pdfPos = 1 / lensArea;
    *pdfDir = 1 / (A * cosTheta * cosTheta * cosTheta);
}

//Spectrum PerspectiveCamera::Sample_Wi(const Interaction &ref, const Point2f &u,
//    Vector3f *wi, Float *pdf,
//    Point2f *pRaster,
//    VisibilityTester *vis) const {
//    // Uniformly sample a lens interaction _lensIntr_
//    Point2f pLens = lensRadius * ConcentricSampleDisk(u);
//    Point3f pLensWorld = CameraToWorld(ref.time, Point3f(pLens.x, pLens.y, 0));
//    Interaction lensIntr(pLensWorld, ref.time, medium);
//    lensIntr.n = Normal3f(CameraToWorld(ref.time, Vector3f(0, 0, 1)));
//
//    // Populate arguments and compute the importance value
//    *vis = VisibilityTester(ref, lensIntr);
//    *wi = lensIntr.p - ref.p;
//    Float dist = wi->Length();
//    *wi /= dist;
//
//    // Compute PDF for importance arriving at _ref_
//
//    // Compute lens area of perspective camera
//    Float lensArea = lensRadius != 0 ? (Pi * lensRadius * lensRadius) : 1;
//    *pdf = (dist * dist) / (AbsDot(lensIntr.n, *wi) * lensArea);
//    return We(lensIntr.SpawnRay(-*wi), pRaster);
//}