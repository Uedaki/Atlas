#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Film.h"
#include "atlas/core/Medium.h"
#include "atlas/core/Points.h"
#include "atlas/core/Transform.h"

namespace atlas
{
	struct Ray;

	struct CameraSample
	{
		Point2f pFilm;
		Point2f pLens;
		Float time;
	};

    class Camera
    {
    public:
        ATLAS Camera(const Transform &CameraToWorld, Float shutterOpen,
                        Float shutterClose, Film *film, const Medium *medium);
        ATLAS virtual ~Camera();
        virtual Float generateRay(const CameraSample &sample, Ray &ray) const = 0;
        ATLAS virtual Float generateRayDifferential(const CameraSample &sample, RayDifferential *rd) const;
        ATLAS virtual Spectrum we(const Ray &ray, Point2f *pRaster2 = nullptr) const;
        ATLAS virtual void pdfWe(const Ray &ray, Float *pdfPos, Float *pdfDir) const;
        // virtual Spectrum sampleWi(const Interaction &ref, const Point2f &u, Vec3f *wi, Float *pdf, Point2f *pRaster, VisibilityTester *vis) const;

        Transform CameraToWorld;
        const Float shutterOpen;
        const Float shutterClose;
        Film *film;
        const Medium *medium;
    };

    class ProjectiveCamera : public Camera
    {
    public:
        ProjectiveCamera(const Transform &CameraToWorld,
            const Transform &CameraToScreen,
            const Bounds2f &screenWindow, Float shutterOpen,
            Float shutterClose, Float lensr, Float focald, Film *film,
            const Medium *medium)
            : Camera(CameraToWorld, shutterOpen, shutterClose, film, medium),
            CameraToScreen(CameraToScreen)
        {
            lensRadius = lensr;
            focalDistance = focald;

            ScreenToRaster =
                scale((Float)film->resolution.x, (Float)film->resolution.y, (Float)1) *
                scale(1 / (screenWindow.max.x - screenWindow.min.x),
                    1 / (screenWindow.min.y - screenWindow.max.y), (Float)1) *
                translate(Vec3f(-screenWindow.min.x, -screenWindow.max.y, (Float)0));
            RasterToScreen = ScreenToRaster.inverse();
            RasterToCamera = CameraToScreen.inverse() * RasterToScreen;
        }

    protected:
        Transform CameraToScreen;
        Transform RasterToCamera;
        Transform ScreenToRaster;
        Transform RasterToScreen;
        Float lensRadius;
        Float focalDistance;
    };
}