#pragma once

#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Medium.h"
#include "atlas/core/Primitive.h"
#include "atlas/core/Shape.h"

namespace atlas
{
    class Material;

    class GeometricPrimitive : public Primitive
    {
    public:
        GeometricPrimitive(std::shared_ptr<Shape> s,
#if defined(SHADING)
            std::shared_ptr<Material> m
#else
            std::shared_ptr<Material> m
#endif
        )
            : shape(s), material(m)
        {}

        GeometricPrimitive(std::shared_ptr<Shape> s,
            std::shared_ptr<AreaLight> l)
            : shape(s), areaLight(l)
        {}

        ~GeometricPrimitive() override {}
        ATLAS Bounds3f worldBound() const override;

        ATLAS bool intersect(const Ray &r, SurfaceInteraction &) const override;
        ATLAS bool intersectP(const Ray &r) const override;

        ATLAS void intersect(const Payload &p, std::vector<SurfaceInteraction> &, std::vector<Float> &) const override;

//        void intersect(const ConeRay &r, SurfaceInteraction *) const override;
//        void intersectP(const ConeRay &r) const override;
//
//#ifdef _USE_SIMD
//        S4Bool intersect(const S4Ray &ray, S4SurfaceInteraction &intersection) const override;
//        S4Bool intersectP(const S4Ray &ray) const override;
//
//        void intersect(const S4ConeRay &r, S4SurfaceInteraction *) const override;
//        void intersectP(const S4ConeRay &r) const override;
//#endif

        ATLAS const AreaLight *getAreaLight() const override;
        ATLAS const Material *getMaterial() const override;

        ATLAS void computeScatteringFunctions(SurfaceInteraction &isect, TransportMode mode, bool allowMultipleLobes) const override;

    private:
        std::shared_ptr<Shape> shape;
        std::shared_ptr<Material> material;

        std::shared_ptr<AreaLight> areaLight;
        MediumInterface mediumInterface;
    };
}