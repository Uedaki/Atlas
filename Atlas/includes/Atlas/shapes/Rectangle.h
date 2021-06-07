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
		{}

		ATLAS Bounds3f objectBound() const override;

		ATLAS bool intersect(const Ray &ray, Float &tHit, SurfaceInteraction &intersection, bool testAlphaTexture) const override;
		ATLAS bool intersectP(const Ray &ray, bool testAlphaTexture) const override;

		ATLAS Float area() const override;

		ATLAS Interaction sample(const Point2f &u, Float &pdf) const override;
		ATLAS Interaction sample(const Interaction &ref, const Point2f &u, Float &pdf) const override;

		ATLAS Float pdf(const Interaction &ref, const Vec3f &wi) const override;

	private:
	};
}