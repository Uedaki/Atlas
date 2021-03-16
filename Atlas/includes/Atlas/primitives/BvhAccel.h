#pragma once

#include <vector>
#include <memory>

#include "atlas/AtlasLibHeader.h"
#include "atlas/primitives/Aggregate.h"

namespace atlas
{
    struct BvhBuildNode;
    struct BvhPrimitiveInfo;
    struct LinearBvhNode;

	class BvhAccel : public Aggregate
	{
	public:
        ATLAS BvhAccel(const std::vector<std::shared_ptr<Primitive>> &p);

        ATLAS bool intersect(const Ray &r, SurfaceInteraction &) const override;
        ATLAS bool intersectP(const Ray &r) const override;

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

        Bounds3f worldBound() const override { return (Bounds3f()); }

    private:
        std::vector<std::shared_ptr<Primitive>> primitives;
        LinearBvhNode *nodes = nullptr;

        ATLAS BvhBuildNode *recursiveBuild(std::vector<BvhPrimitiveInfo> &primitiveInfo, int32_t start, int32_t end, int32_t &totalNodes,
                                            std::vector<std::shared_ptr<Primitive>> &orderedPrims);
        ATLAS int32_t flattenBvhTree(BvhBuildNode *node, int32_t &offset);
	};
}