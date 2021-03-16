#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Shape.h"
#include "atlas/core/Points.h"
#ifdef _USE_SIMD
#include "atlas/core/simd/Simd.h"
#include "atlas/core/simd/SRay.h"
#include "atlas/core/simd/SSurfaceInteraction.h"
#endif

namespace atlas
{
	class Sphere : public Shape
	{
	public:
		struct Info : public Shape::Info
		{
			//Transform *Shape::objectToWorld = nullptr;
			//Transform *Shape::worldToObject = nullptr;
			//bool Shape::reverseOrientation = false;
			Float radius = 1.;
			Float zMin = -1.;
			Float zMax = 1.;
			Float phiMax = 360.;
		};

		ATLAS static std::shared_ptr<Shape> createShape(const Info &info);

		Sphere(const Info &info)
			: Shape(info)
			, radius(info.radius)
			, zMin(clamp(std::min(info.zMin, info.zMax), -radius, radius))
			, zMax(clamp(std::max(info.zMin, info.zMax), -radius, radius))
			, thetaMin(std::acos(clamp(std::min(info.zMin, info.zMax) / radius, -1, 1)))
			, thetaMax(std::acos(clamp(std::max(info.zMin, info.zMax) / radius, -1, 1)))
			, phiMax(radians(clamp(info.phiMax, 0, 360)))
		{}

		Sphere(const Transform &objectToWorld, const Transform &worldToObject,
			bool reverseOrientation, Float radius, Float zMin, Float zMax,
			Float phiMax)
			: Shape(objectToWorld, worldToObject, reverseOrientation)
			, radius(radius)
			, zMin(clamp(std::min(zMin, zMax), -radius, radius))
			, zMax(clamp(std::max(zMin, zMax), -radius, radius))
			, thetaMin(std::acos(clamp(std::min(zMin, zMax) / radius, -1, 1)))
			, thetaMax(std::acos(clamp(std::max(zMin, zMax) / radius, -1, 1)))
			, phiMax(radians(clamp(phiMax, 0, 360)))
		{}

		ATLAS Bounds3f objectBound() const override;

		ATLAS bool intersect(const Ray &ray, Float &tHit, SurfaceInteraction &intersection, bool testAlphaTexture) const override;
		ATLAS bool intersectP(const Ray &ray, bool testAlphaTexture) const override;

//#ifdef _USE_SIMD
//		S4Bool intersect(const S4Ray &ray, S4SurfaceInteraction &intersection) const override;
//		S4Bool intersectP(const S4Ray &ray) const override;
//#endif

		ATLAS Float area() const override;

		ATLAS Interaction sample(const Point2f &u, Float &pdf) const override;
		ATLAS Interaction sample(const Interaction &ref, const Point2f &u, Float &pdf) const override;

		ATLAS Float pdf(const Interaction &ref, const Vec3f &wi) const override;

	private:
		Float radius;
		Float zMin;
		Float zMax;
		Float thetaMin;
		Float thetaMax;
		Float phiMax;
	};

	typedef Sphere::Info SphereInfo;
}