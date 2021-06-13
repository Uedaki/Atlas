#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Shape.h"
#include "atlas/core/Points.h"

namespace atlas
{
	class Rectangle : public Shape
	{
	public:
		ATLAS static std::shared_ptr<Shape> createRectangle();

		Rectangle(const Transform &objectToWorld, const Transform &worldToObject,
			bool reverseOrientation)
			: Shape(objectToWorld, worldToObject, reverseOrientation)
			, p0(objectToWorld(Point3f(-1, -1, 0)))
			, p1(objectToWorld(Point3f(1, -1, 0)))
			, p2(objectToWorld(Point3f(-1, 1, 0)))
		{}

		ATLAS Bounds3f objectBound() const override;
		ATLAS Bounds3f worldBound() const override;

		ATLAS bool intersect(const Ray &ray, Float &tHit, SurfaceInteraction &intersection, bool testAlphaTexture) const override;
		ATLAS bool intersectP(const Ray &ray, bool testAlphaTexture) const override;

		ATLAS Float area() const override;

		ATLAS Interaction sample(const Point2f &u, Float &pdf) const override;
		ATLAS Interaction sample(const Interaction &ref, const Point2f &u, Float &pdf) const override;

		ATLAS Float pdf(const Interaction &ref, const Vec3f &wi) const override;

	private:
		Point3f p0;
		Point3f p1;
		Point3f p2;
	};
}