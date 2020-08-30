#pragma once

#include <array>
#include <cstdint>

#include "BinFile.h"
#include "CompactRay.h"
#include "BatchManager.h"

class GlobalBins
{
public:
	GlobalBins(BatchManager &manager);

	void open();
	void map();
	void unmap();

	void feed(uint8_t binIndex, const std::array<CompactRay, 256> &bin, uint32_t size);

	bool purge(Batch &batch);
private:
	std::array<BinFile, 6> bins;
};