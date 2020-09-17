#pragma once

#include "Atlas/rendering/Ray.h"

#include <array>

#include "CompactRay.h"

#include "GlobalBins.h"
#include "BinSize.h"

class LocalBins
{
public:
	LocalBins(GlobalBins &c);
	void feed(const atlas::rendering::Ray &ray, const CompactRay &compatchRay);
	void flush();

private:
	GlobalBins &collector;
	std::array<uint32_t, 6> binSizes = {0, 0, 0, 0, 0, 0};
	std::array<std::array<CompactRay, LOCAL_BIN_SIZE>, 6> bins;
};