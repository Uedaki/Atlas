#include "pch.h"
#include "GlobalBins.h"

GlobalBins::GlobalBins(BatchManager &manager)
{
	for (auto &bin : bins)
	{
		bin.setManager(manager);
	}
}

void GlobalBins::open()
{
	for (auto &bin : bins)
	{
		bin.open();
	}
}

void GlobalBins::map()
{
	for (auto &bin : bins)
	{
		bin.map();
	}
}

void GlobalBins::unmap()
{
	for (auto &bin : bins)
	{
		bin.unmap();
	}
}

void GlobalBins::feed(uint8_t binIndex, const std::array<CompactRay, LOCAL_BIN_SIZE> &rays, uint32_t size)
{
	//printf("add local bin to %d bin\n", binIndex);
	bins[binIndex].feed(rays, size);
}

bool GlobalBins::purge(Batch &batch)
{
	uint32_t fullestBin = 0;
	for (uint8_t i = 1; i < 6; i++)
	{
		if (bins[i].getSize() > bins[fullestBin].getSize())
			fullestBin = i;
	}
	if (bins[fullestBin].getSize() > 0)
	{
		bins[fullestBin].map();
		bins[fullestBin].purge(batch);
		return (true);
	}
	else
		return (false);
}

BinFile *GlobalBins::getBin()
{
	uint32_t fullestBin = 0;
	for (uint8_t i = 1; i < 6; i++)
	{
		if (bins[i].getSize() > bins[fullestBin].getSize())
			fullestBin = i;
	}
	if (bins[fullestBin].getSize() > 0)
	{
		return (&bins[fullestBin]);
	}
	else
		return (nullptr);
}