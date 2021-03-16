#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Geometry.h"
#include "atlas/core/Math.h"
#include "atlas/core/RgbSpectrum.h"
#include "atlas/core/Sampling.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
	inline Float cosTheta(const Vec3f &w)
	{
		return w.z;
	}

	inline Float cos2Theta(const Vec3f &w)
	{
		return w.z * w.z;
	}

	inline Float absCosTheta(const Vec3f &w)
	{
		return std::abs(w.z);
	}

	inline Float sin2Theta(const Vec3f &w)
	{
		return std::max((Float)0, (Float)1 - cos2Theta(w));
	}

	inline Float sinTheta(const Vec3f &w)
	{
		return std::sqrt(sin2Theta(w));
	}

	inline Float tanTheta(const Vec3f &w)
	{
		return sinTheta(w) / cosTheta(w);
	}

	inline Float tan2Theta(const Vec3f &w)
	{
		return sin2Theta(w) / cos2Theta(w);
	}

	inline Float cosPhi(const Vec3f &w)
	{
		Float sinT = sinTheta(w);
		return (sinT == 0) ? 1 : clamp(w.x / sinT, -1.f, 1.f);
	}
	inline Float sinPhi(const Vec3f &w)
	{
		Float sinT = sinTheta(w);
		return (sinT == 0) ? 0 : clamp(w.y / sinT, -1.f, 1.f);
	}

	inline Float cos2Phi(const Vec3f &w)
	{
		return cosPhi(w) * cosPhi(w);
	}

	inline Float sin2Phi(const Vec3f &w)
	{
		return sinPhi(w) * sinPhi(w);
	}

	inline Float cosDPhi(const Vec3f &wa, const Vec3f &wb)
	{
		return clamp((wa.x * wb.x + wa.y * wb.y) /
			std::sqrt((wa.x * wa.x + wa.y * wa.y) *
				(wb.x * wb.x + wb.y * wb.y)), -1.f, 1.f);
	}

	ATLAS Float fresnelDielectric(Float cosThetaI, Float etaI, Float etaT);

	ATLAS Spectrum fresnelConductor(Float cosThetaI, const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k);

	inline Vec3f reflect(const Vec3f &wo, const Vec3f &n)
	{
		return (-wo + 2 * dot(wo, n) * n);
	}

	inline bool refract(const Vec3f &wi, const Normal &n, Float eta, Vec3f &wt)
	{
		Float cosThetaI = dot(n, wi);
		Float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
		Float sin2ThetaT = eta * eta * sin2ThetaI;
		Float cosThetaT = std::sqrt(1 - sin2ThetaT);
		if (sin2ThetaT >= 1)
			return (false);
		wt = eta * -wi + (eta * cosThetaI - cosThetaT) * n;
		return (true);
	}
}