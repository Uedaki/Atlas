#include "GenerateFirstRays.h"


bool atlas::task::GenerateFirstRays::preExecute()
{
	zOrderMax = posToZOrderIndex(data.resolution.x, data.resolution.y);
	data.batchManager->openBins();
	return (true);
}

void atlas::task::GenerateFirstRays::execute()
{
	thread_local std::array<LocalBin, 6> localBins =
	{ LocalBin(data.localBinSize), LocalBin(data.localBinSize), LocalBin(data.localBinSize),
	LocalBin(data.localBinSize), LocalBin(data.localBinSize), LocalBin(data.localBinSize) };
	std::unique_ptr<Sampler> sampler(data.sampler->clone(1));

	while (true)
	{
		const uint64_t offset = zOrderPos.fetch_add(zOrderStep);
		if (offset >= zOrderMax)
			break;

		for (uint64_t i = offset; i < offset + zOrderStep; i++)
		{
			uint32_t x;
			uint32_t y;
			zOrderIndexToPos(i, x, y);
			if (x < (uint32_t)data.resolution.x && y < (uint32_t)data.resolution.y)
			{
				sampler->startPixel(Point2i(x, y), true);
				for (uint32_t s = 0; s < data.spp; s++)
				{
					atlas::Ray r;
					atlas::CameraSample cs = sampler->getCameraSample(Point2i(x, y));
					data.camera->generateRay(cs, r);

					const uint8_t vectorIndex = abs(r.dir).maxDimension();
					const bool isNegative = std::signbit(r.dir[vectorIndex]);
					const uint8_t index = vectorIndex * 2 + isNegative;
					if (localBins[index].feed(CompactRay(r, x + y * data.resolution.x, s, 0)))
						data.batchManager->feed(index, localBins[index]);

					sampler->startNextSample();
				}
			}
		}
	}

	for (uint32_t i = 0; i < localBins.size(); i++)
	{
		data.batchManager->feed(i, localBins[i]);
	}
}

void atlas::task::GenerateFirstRays::postExecute()
{
	data.batchManager->unmapBins();
}