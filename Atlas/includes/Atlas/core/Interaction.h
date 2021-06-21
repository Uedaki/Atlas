#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Geometry.h"
#include "atlas/core/Medium.h"
#include "atlas/core/Points.h"
#include "atlas/core/Ray.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
	class Material;

	struct Interaction
	{
		Point3f p;
		Float time;
		Vec3f pError;
		Vec3f wo;
		Normal n;
		MediumInterface mediumInterface;

		Interaction()
			: time(0)
		{}

		Interaction(const Point3f &p, const Normal &n, const Vec3f &pError,
			const Vec3f &wo, Float time,
			const MediumInterface &mediumInterface)
			: p(p)
			, time(time)
			, pError(pError)
			, wo(normalize(wo))
			, n(n)
			, mediumInterface(mediumInterface)
		{}

		Interaction(const Point3f &p, const Vec3f &wo, Float time, const MediumInterface &mediumInterface)
			: p(p), time(time), wo(wo), mediumInterface(mediumInterface)
		{}
		
		Interaction(const Point3f &p, Float time, const MediumInterface &mediumInterface)
			: p(p), time(time), mediumInterface(mediumInterface)
		{}

		bool isSurfaceInteraction() const { return (n != Normal()); }

		Ray spawnRay(const Vec3f &d) const
		{
			Point3f o = offsetRayOrigin(p, pError, n, d);
			return (Ray(o, d, INFINITY, time, getMedium()));
		}

		Ray spawnRayTo(const Point3f &p2) const
		{
			Point3f origin = offsetRayOrigin(p, pError, n, p2 - p);
			Vec3f d = p2 - p;
			return (Ray(origin, d, 1 - SHADOW_EPSILON, time, getMedium()));
		}

		Ray spawnRayTo(const Interaction &it) const
		{
			Point3f origin = offsetRayOrigin(p, pError, n, it.p - p);
			Point3f target = offsetRayOrigin(p, pError, n, origin - p);
			Vec3f d = target - origin;
			return (Ray(origin, d, 1 - SHADOW_EPSILON, time, getMedium()));
		}

		const Medium *getMedium() const
		{
			return (mediumInterface.inside);
		}
	};

	struct MediumInteraction : public Interaction
	{
		MediumInteraction()
			: phase(nullptr)
		{}

		MediumInteraction(const Point3f &p, const Vec3f &wo, Float time,
			const Medium *medium, const PhaseFunction *phase)
			: Interaction(p, wo, time, medium), phase(phase)
		{}
		
		bool isValid() const
		{
			return phase != nullptr;
		}

		const PhaseFunction *phase;
	};

	struct BSDF;
	class Primitive;
	class Shape;

	struct SurfaceInteraction : public Interaction
	{
		Point2f uv;
		Vec3f dpdu;
		Vec3f dpdv;
		Normal dndu;
		Normal dndv;
		const Shape *shape = nullptr;
		struct
		{
			Normal n;
			Vec3f dpdu;
			Vec3f dpdv;
			Normal dndu;
			Normal dndv;
		} shading;
		const Primitive *primitive = nullptr;
		Material *material = nullptr;
		BSDF *bsdf = nullptr;

		mutable Vec3f dpdx;
		mutable Vec3f dpdy;
		mutable Float dudx = 0;
		mutable Float dvdx = 0;
		mutable Float dudy = 0;
		mutable Float dvdy = 0;

		int faceIndex = 0;

		SurfaceInteraction() = default;

		ATLAS SurfaceInteraction(const Point3f &p, const Vec3f &pError,
			const Point2f &uv, const Vec3f &wo,
			const Vec3f &dpdu, const Vec3f &dpdv,
			const Normal &dndu, const Normal &dndv, Float time,
			const Shape *sh,
			int faceIndex = 0);

		ATLAS void setShadingGeometry(const Vec3f &dpdu, const Vec3f &dpdv,
			const Normal &dndu, const Normal &dndv,
			bool orientationIsAuthoritative);

		ATLAS void computeScatteringFunctions(
			const RayDifferential &ray, bool allowMultipleLobes = false,
			TransportMode mode = TransportMode::Radiance);

		ATLAS void computeDifferentials(const RayDifferential &r) const;

		ATLAS Spectrum le(const Vec3f &w) const;
	};
}