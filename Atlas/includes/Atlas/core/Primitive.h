#pragma once

#include "atlas/core/Bounds.h"
#include "atlas/core/Ray.h"
#include "atlas/core/Material.h"
#ifdef _USE_SIMD
#include "atlas/core/simd/Simd.h"
#include "atlas/core/simd/SRay.h"
#include "atlas/core/simd/SSurfaceInteraction.h"
#endif

namespace atlas
{
    class AreaLight
    {

    };

    struct SurfaceInteraction;

    class Primitive
    {
    public:
        virtual ~Primitive() {}
        virtual Bounds3f worldBound() const = 0;
 
        virtual bool intersect(const Ray &r, SurfaceInteraction &) const = 0;
        virtual bool intersectP(const Ray &r) const = 0;

//        virtual void intersect(const ConeRay &r, SurfaceInteraction *) const = 0;
//        virtual void intersectP(const ConeRay &r) const = 0;
//        
//#ifdef _USE_SIMD
//        virtual S4Bool intersect(const S4Ray &ray, S4SurfaceInteraction &intersection) const = 0;
//        virtual S4Bool intersectP(const S4Ray &ray) const = 0;
//
//        virtual void intersect(const S4ConeRay &r, S4SurfaceInteraction *) const = 0;
//        virtual void intersectP(const S4ConeRay &r) const = 0;
//#endif

        virtual const AreaLight *getAreaLight() const = 0;
        virtual const Material *getMaterial() const = 0;

        virtual void computeScatteringFunctions(SurfaceInteraction &isect, TransportMode mode, bool allowMultipleLobes) const = 0;
    };
}