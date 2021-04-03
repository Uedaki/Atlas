#pragma once

#include "Atlas/Atlas.h"
#include "Atlas/core/Points.h"
#include "Atlas/core/Ray.h"
#include "Atlas/core/RgbSpectrum.h"
#include "Atlas/core/Vectors.h"

namespace atlas
{
	inline Vector2<float> octWrap(Vector2<float> v)
	{
		return (Vec2f((1.0 - std::abs(v.y)) * (v.x >= 0.0 ? 1.0 : -1.0),
			(1.0 - std::abs(v.x)) * (v.y >= 0.0 ? 1.0 : -1.0)));
	}

	inline Vector2<float> octEncode(Vec3f dn)
	{
		Vector3<float> n(dn);

		Vector3<float> r = n / (std::abs(n.x) + std::abs(n.y) + std::abs(n.z));
		Vector2<float> c = r.z >= 0.0 ? Vector2<float>(r.x, r.y) : octWrap(Vector2<float>(r.x, r.y));
		c.x = c.x * 0.5f + 0.5f;
		c.y = c.y * 0.5f + 0.5f;
		return (c);
	}

	inline Vec3f octDecode(Vector2<float> f)
	{
		f = f * 2.0f - 1.0f;

		// https://twitter.com/Stubbesaurus/status/937994790553227264
		Vector3<float> n(f.x, f.y, 1.0 - std::abs(f.x) - std::abs(f.y));
		float t = std::max(-n.z, 0.f);
		n.x += n.x >= 0.0 ? -t : t;
		n.y += n.y >= 0.0 ? -t : t;
		return (normalize(n));
	}

#pragma pack(push, 4)
	struct CompactRay
	{
		Point3<float> origin;
		Vector2<float> direction;
		uint32_t weight;
		uint32_t pixelID;
		uint16_t sampleID;
		uint16_t depth; // suppose to be ray diameter
		float tNear;

		CompactRay() = default;
		CompactRay(const Ray &ray, const Spectrum &color, uint32_t pixel, uint32_t sample, uint32_t depth, float tNear = 0)
			: origin(ray.origin)
			, direction(octEncode(ray.dir))
			, weight(toRgb9e5(color))
			, pixelID(pixel)
			, sampleID(sample)
			, depth(depth)
			, tNear(tNear)
		{}

		CompactRay(const Ray &ray, uint32_t pixel, uint32_t sample, float tNear = 0)
			: origin(ray.origin)
			, direction(octEncode(ray.dir))
			, weight(toRgb9e5(Spectrum(1.f)))
			, pixelID(pixel)
			, sampleID(sample)
			, depth(0)
			, tNear(tNear)
		{}

		//void extract(NRay &ray)
		//{
		//	ray.origin = origin;
		//	ray.dir = octDecode(direction);
		//	ray.weight = toColor(weight);
		//	ray.pixelID = pixelID;
		//	ray.sampleID = sampleID;
		//	ray.depth = depth;
		//	ray.tNear = tNear;
		//}

		//void compress(const NRay &ray)
		//{
		//	origin = ray.origin;
		//	direction = octEncode(ray.dir);
		//	weight = toRgb9e5(ray.weight);
		//	pixelID = ray.pixelID;
		//	sampleID = ray.sampleID;
		//	depth = ray.depth;
		//	tNear = ray.tNear;
		//}
	};
#pragma pack(pop)
}