#pragma once

#include <immintrin.h>

#include "Acheron.h"
#include "Bin.h"
#include "CompactRay.h"
#include "ThreadPool.h"

namespace atlas
{
	namespace task
	{
		class GenerateFirstRays : public ThreadedTask
		{
		public:
			static constexpr uint64_t zOrderStep = 516;

			GenerateFirstRays(Acheron &renderer, const Camera &camera, uint32_t spp)
				: renderer(renderer), camera(camera), spp(spp)
			{}

			void preExecute() override
			{
				zOrderMax = posToZOrderIndex(renderer.resolution.x, renderer.resolution.y);
				renderer.manager.openBins();
			}

			void execute() override
			{
				thread_local std::array<LocalBin, 6> localBins;

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
						if (x < renderer.resolution.x && y < renderer.resolution.y)
						{
							for (uint32_t s = 0; s < spp; s++)
							{
								atlas::Ray r;
								atlas::CameraSample cs = renderer.getSampler().getCameraSample(Point2i(x, y));
								camera.generateRay(cs, r);

								const uint8_t vectorIndex = abs(r.dir).maxDimension();
								const bool isNegative = std::signbit(r.dir[vectorIndex]);
								const uint8_t index = vectorIndex * 2 + isNegative;
								if (localBins[index].feed(CompactRay(r, x + y * renderer.resolution.x, s, 0)))
									renderer.manager.feed(localBins[i]);
							}
						}
					}
				}

				for (uint32_t i = 0; i < localBins.size(); i++)
				{
					renderer.manager.feed(localBins[i]);
				}
			}

			void postExecute() override
			{
				renderer.manager.unmapBins();
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
			Acheron &renderer;
			const Camera &camera;

			std::atomic<uint64_t> zOrderPos = 0;
			uint64_t zOrderMax = 0;
			uint32_t spp;
		};
	}
}