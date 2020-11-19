#pragma once

#include <glm/glm.hpp>

#include "Atlas/rendering/Ray.h"

#include "utils.h"

#pragma pack(push, 4)
struct CompactRay
{
	glm::vec3 origin;
	glm::vec2 direction;
	uint32_t weight;
	uint32_t pixelID;
	uint16_t sampleID;
	uint16_t depth; // suppose to be ray diameter
	float tNear;

	CompactRay() = default;

	CompactRay(const atlas::rendering::Ray &ray, const glm::vec3 &color, uint32_t pixel, uint32_t sample, uint32_t depth, float tNear = 0)
		: origin(ray.origin)
		, direction(octEncode(ray.dir))
		, weight(toRgb9e5(color))
		, pixelID(pixel)
		, sampleID(sample)
		, depth(depth)
		, tNear(tNear)
	{}

	CompactRay(const atlas::rendering::Ray &ray, uint32_t pixel, uint32_t sample, float tNear = 0)
		: origin(ray.origin)
		, direction(octEncode(ray.dir))
		, weight(toRgb9e5(glm::vec3(1, 1, 1)))
		, pixelID(pixel)
		, sampleID(sample)
		, depth(0)
		, tNear(tNear)
	{}

	void extract(NRay &ray)
	{
		ray.origin = origin;
		ray.dir = octDecode(direction);
		ray.weight = toColor(weight);
		ray.pixelID = pixelID;
		ray.sampleID = sampleID;
		ray.depth = depth;
		ray.tNear = tNear;
	}

	void compress(const NRay &ray)
	{
		origin = ray.origin;
		direction = octEncode(ray.dir);
		weight = toRgb9e5(ray.weight);
		pixelID = ray.pixelID;
		sampleID = ray.sampleID;
		depth = ray.depth;
		tNear = ray.tNear;
	}
};
#pragma pack(pop)