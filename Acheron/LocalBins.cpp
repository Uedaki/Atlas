#include "pch.h"
#include "LocalBins.h"

#include "utils.h"

LocalBins::LocalBins(GlobalBins &c)
	: collector(c)
{}

void LocalBins::feed(const atlas::rendering::Ray &ray, const CompactRay &compactRay)
{
	const uint8_t vectorIndex = maxIdx(glm::abs(ray.dir));
	const bool isNegative = std::signbit(ray.dir[vectorIndex]);
	const uint8_t index = vectorIndex * 2 + isNegative;
	const uint32_t pos = binSizes[index];
	bins[index][pos] = compactRay;
	binSizes[index] += 1;
	//printf("Local bin add [%d, %d]\n", index, pos);
	if (pos + 1 == LOCAL_BIN_SIZE)
	{
		collector.feed(index, bins[index], LOCAL_BIN_SIZE);
		binSizes[index] = 0;
	}
}

void LocalBins::flush()
{
	for (uint8_t i = 0; i < binSizes.size(); i++)
	{
		if (binSizes[i])
			collector.feed(i, bins[i], binSizes[i]);
	}
}