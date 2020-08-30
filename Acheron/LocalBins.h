#pragma once

#include "Atlas/rendering/Ray.h"

#include <array>

#include "CompactRay.h"

#include "GlobalBins.h"

class LocalBins
{
	#define BinSize 256
public:
	LocalBins(GlobalBins &c);
	void feed(const atlas::rendering::Ray &ray, const CompactRay &compatchRay);
	void flush();

private:
	GlobalBins &collector;
	std::array<uint32_t, 6> binSizes = {0, 0, 0, 0, 0, 0};
	std::array<std::array<CompactRay, BinSize>, 6> bins;
};