#include "atlas/core/Camera.h"

using namespace atlas;

Camera::Camera(const Transform &CameraToWorld, Float shutterOpen,
    Float shutterClose, Film *film, const Medium *medium)
    : CameraToWorld(CameraToWorld)
    , shutterOpen(shutterOpen)
    , shutterClose(shutterClose)
    , film(film)
    , medium(medium)
{

}

Camera::~Camera() {
    //delete film;
}

Float Camera::generateRayDifferential(const CameraSample &sample, RayDifferential *rd) const
{
    Float wt = generateRay(sample, *rd);
    if (wt == 0)
        return 0;

    Float wtx;
    for (Float eps : { (Float).05, (Float)-.05 })
    {
        CameraSample sshift = sample;
        sshift.pFilm.x += eps;
        Ray rx;
        wtx = generateRay(sshift, rx);
        rd->rxOrigin = rd->origin + (rx.origin - rd->origin) / eps;
        rd->rxDirection = rd->dir + (rx.dir - rd->dir) / eps;
        if (wtx != 0)
            break;
    }
    if (wtx == 0)
        return 0;

    Float wty;
    for (Float eps : { (Float).05, (Float)-.05 })
    {
        CameraSample sshift = sample;
        sshift.pFilm.y += eps;
        Ray ry;
        wty = generateRay(sshift, ry);
        rd->ryOrigin = rd->origin + (ry.origin - rd->origin) / eps;
        rd->ryDirection = rd->dir + (ry.dir - rd->dir) / eps;
        if (wty != 0)
            break;
    }
    if (wty == 0)
        return 0;

    rd->hasDifferentials = true;
    return wt;
}

Spectrum Camera::we(const Ray &ray, Point2f *raster) const
{
    return Spectrum(0.f);
}

void Camera::pdfWe(const Ray &ray, Float *pdfPos, Float *pdfDir) const
{

}

//Spectrum Camera::sampleWi(const Interaction &ref, const Point2f &u,
//    Vec3f *wi, Float *pdf, Point2f *pRaster,
//    VisibilityTester *vis) const {
//    LOG(FATAL) << "Camera::Sample_Wi() is not implemented!";
//    return Spectrum(0.f);
//}