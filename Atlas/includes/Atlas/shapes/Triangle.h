#pragma once

#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Shape.h"
#include "atlas/core/TriangleMesh.h"
#include "atlas/core/Vectors.h"
#include "atlas/core/Geometry.h"
#include "atlas/core/Sampling.h"

namespace atlas
{
	class Triangle : public Shape
	{
	public:
        ATLAS Triangle(const Transform &objectToWorld, const Transform &worldToObject, bool reverseOrientation, const std::shared_ptr<TriangleMesh> &mesh, uint32_t triNumber);

        ATLAS Bounds3f objectBound() const override;
		ATLAS Bounds3f worldBound() const override;

		ATLAS bool intersect(const Ray &ray, Float &tHit, SurfaceInteraction &isect, bool testAlphaTexture = true) const override;
        ATLAS bool intersectP(const Ray &ray, bool testAlphaTexture = true) const override;
        ATLAS Float area() const override;
        ATLAS Interaction sample(const Point2f &u, Float &pdf) const override;

	private:
		std::shared_ptr<TriangleMesh> mesh;
		const uint32_t *v;
        uint32_t faceIndex;

        inline void GetUVs(Point2f uv[3]) const
        {
            if (mesh->uv)
            {
                uv[0] = mesh->uv[v[0]];
                uv[1] = mesh->uv[v[1]];
                uv[2] = mesh->uv[v[2]];
            }
            else
            {
                uv[0] = Point2f(0, 0);
                uv[1] = Point2f(1, 0);
                uv[2] = Point2f(1, 1);
            }
        }
	};

    ATLAS std::vector<std::shared_ptr<Shape>> createTriangleMesh(const Transform &objectToWorld, const Transform &worldToObject, bool reverseOrientation, uint32_t nTriangles, const uint32_t *vertexIndices, uint32_t nVertices, const Point3f *p
        , const Vec3f *s, const Normal *n, const Point2f *uv);
}