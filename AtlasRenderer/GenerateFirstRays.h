#pragma once

#include <immintrin.h>

#include "Acheron.h"
#include "Bin.h"
#include "CompactRay.h"
#include "ThreadPool.h"
#include "LocalBin.h"

namespace atlas
{
	namespace task
	{
		class GenerateFirstRays : public ThreadedTask
		{
		public:
			static constexpr uint64_t zOrderStep = 1024;

			struct Data
			{
				Point2i resolution;
				uint32_t spp;

				uint32_t localBinSize = 512;

				const Camera *camera = nullptr;
				Sampler *sampler = nullptr;
				BatchManager *batchManager = nullptr;
			};

			GenerateFirstRays(Data &data)
				: data(data)
			{}

			bool preExecute() override
			{
				zOrderMax = posToZOrderIndex(data.resolution.x, data.resolution.y);
				data.batchManager->openBins();
				return (true);
			}

			void execute() override
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
							sampler->startPixel(Point2i(x, y));
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

			void postExecute() override
			{
				data.batchManager->unmapBins();
			}

			uint64_t posToZOrderIndex(uint32_t x, uint32_t y)
			{
				return (_pdep_u32(x, 0x55555555) | _pdep_u32(y, 0xaaaaaaaa));
			}

			uint64_t zOrderIndexToPos(uint64_t index, uint32_t &x, uint32_t &y)
			{
				x = static_cast<uint32_t>(_pext_u64(index, 0x5555555555555555));
				y = static_cast<uint32_t>(_pext_u64(index, 0xaaaaaaaaaaaaaaaa));
				return (index);
			}

		private:
			Data data;

			std::atomic<uint64_t> zOrderPos = 0;
			uint64_t zOrderMax = 0;
		};
	}
}