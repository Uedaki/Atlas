#pragma once

#include <glm/glm.hpp>

#include <cstdint>

#include <algorithm>

# ifdef ACHERON_EXPORTS
#   define ACH  __declspec( dllexport )
# else
#   define ACH __declspec( dllimport )
# endif

struct NRay
{
	glm::vec3 origin;
	glm::vec3 dir;
	glm::vec3 weight;
	uint32_t pixelID;
	uint16_t sampleID;
	uint16_t depth;
	float tNear;
};

uint64_t posToZOrderIndex(uint32_t x, uint32_t y);

uint64_t zOrderIndexToPos(uint64_t index, uint32_t &x, uint32_t &y);

uint32_t toRgb9e5(const glm::vec3 &weight);

glm::vec3 toColor(uint32_t weight);

void copyUintToBit(uint32_t src, uint32_t &dst, uint32_t size, uint32_t offset);

uint32_t copyBitToUint(uint32_t src, uint32_t size, uint32_t offset);

glm::vec2 OctWrap(glm::vec2 v);

ACH glm::vec2 octEncode(glm::vec3 n);
ACH glm::vec3 octDecode(glm::vec2 f);

float maxValue(const glm::vec3 &v);
float maxIdx(const glm::vec3 &v);