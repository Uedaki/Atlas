#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Bounds.h"
#include "atlas/core/Ray.h"
#include "atlas/core/Transform.h"
#include "atlas/core/Interaction.h"
#ifdef _USE_SIMD
#include "atlas/core/simd/Simd.h"
#include "atlas/core/simd/SRay.h"
#include "atlas/core/simd/SSurfaceInteraction.h"
#endif

namespace atlas
{
	class Shape
	{
	public:
		struct Info
		{
			Transform *objectToWorld = nullptr;
			Transform *worldToObject = nullptr;
			bool reverseOrientation = false;
		};

		ATLAS Shape(const Info &info);
		ATLAS Shape(const Transform &objectToWorld, const Transform &worldToObject, bool reverseOrientation);
		virtual ~Shape() = default;

		virtual Bounds3f objectBound() const = 0;
		ATLAS virtual Bounds3f worldBound() const;

		virtual bool intersect(const Ray &ray, Float &tHit, SurfaceInteraction &intersection, bool testAlphaTexture = true) const = 0;
		virtual bool intersectP(const Ray &ray, bool testAlphaTexture = true) const = 0;

//#ifdef _USE_SIMD
//		virtual S4Bool intersect(const S4Ray &ray, S4SurfaceInteraction &intersection) const = 0;
//		virtual S4Bool intersectP(const S4Ray &ray) const = 0;
//#endif

		virtual Float area() const = 0;
		
		virtual Interaction sample(const Point2f &u, Float &pdf) const = 0;
		virtual Float pdf(const Interaction &) const { return (1.f / area()); }

		ATLAS virtual Interaction sample(const Interaction &ref, const Point2f &u, Float &pdf) const;
		ATLAS virtual Float pdf(const Interaction &ref, const Vec3f &wi) const;

		const Transform &objectToWorld;
		const Transform &worldToObject;
		const bool reverseOrientation;
		const bool transformSwapsHandedness;
	};
}