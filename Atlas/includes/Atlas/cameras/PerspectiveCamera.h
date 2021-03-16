#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Camera.h"
#include "atlas/core/Points.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
    class PerspectiveCamera : public ProjectiveCamera
    {
    public:
        ATLAS PerspectiveCamera(const Transform &CameraToWorld,
                                const Bounds2f &screenWindow, Float shutterOpen,
                                Float shutterClose, Float lensRadius, Float focalDistance,
                                Float fov, Film *film, const Medium *medium);
        ATLAS Float generateRay(const CameraSample &sample, Ray &ray) const;
        ATLAS Float generateRayDifferential(const CameraSample &sample,
                                            RayDifferential *ray) const;
        ATLAS Spectrum we(const Ray &ray, Point2f *pRaster2 = nullptr) const;
        ATLAS void pdfWe(const Ray &ray, Float *pdfPos, Float *pdfDir) const;

    private:
        Vec3f dxCamera;
        Vec3f dyCamera;
        Float A;
    };
}