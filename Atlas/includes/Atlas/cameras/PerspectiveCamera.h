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
        struct Info
        {
            Transform *cameraToWorld;
            Bounds2f screenWindow;

            Float shutterOpen;
            Float shutterClose;

            Float lensRadius;
            Float focalDistance;

            Float horizontalFov;
            Float verticalFov;

            Film *film = nullptr;
            Medium *medium = nullptr;
        };

        ATLAS PerspectiveCamera(const Info &info);
        ATLAS PerspectiveCamera(const Transform &CameraToWorld,
            const Bounds2f &screenWindow, Float shutterOpen,
            Float shutterClose, Float lensRadius, Float focalDistance,
            Float fov, Film *film, const Medium *medium);
        ATLAS PerspectiveCamera(const Transform &CameraToWorld,
                                const Bounds2f &screenWindow, Float shutterOpen,
                                Float shutterClose, Float lensRadius, Float focalDistance,
                                Float vFov, Float hFov, Film *film, const Medium *medium);
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

    typedef PerspectiveCamera::Info PerspectiveCameraInfo;

    inline std::ostream &operator<<(std::ostream &os, const PerspectiveCameraInfo &info)
    {
        os << "PerspectiveCameraInfo {\n"
            << "\tCamera to world: " << *info.cameraToWorld << "\n"
            << "\tShutter open: " << info.shutterOpen << "\n"
            << "\tShutter close: " << info.shutterClose << "\n"
            << "\tLens radius: " << info.lensRadius << "\n"
            << "\tFocal distance: " << info.focalDistance << "\n"
            << "\tHorizontal fov: " << info.horizontalFov << "\n"
            << "\tVertical fov: " << info.verticalFov << "\n"
            << "\tFilm ptr: " << (uint64_t)info.film << "\n"
            << "\tMedium ptr: " << (uint64_t)info.medium << "\n"
            << "}\n";
        return (os);
    }
}