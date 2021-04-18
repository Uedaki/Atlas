#pragma once

#include <immintrin.h>

#include "Acheron.h"
#include "Bin.h"
#include "CompactRay.h"
#include "ThreadPool.h"

#include "BSDF.h"
#include "Material.h"

namespace atlas
{
	struct ShadingPack
	{
		uint32_t start;
		uint32_t end;
		sh::Material *material;
	};

	struct Sample
	{
		uint32_t pixelID;
		Spectrum color;
	};

	namespace task
	{
		class Shade : public ThreadedTask
		{
		public:
			struct Data
			{
				uint32_t startingIndex;
				uint32_t maxDepth;

				std::vector<Sample> *samples;
				std::mutex *samplesGuard;

				Batch *batch;
				std::vector<SurfaceInteraction> *interactions;

				std::vector<ShadingPack> *shadingPack;

				Sampler *sampler = nullptr;
				BatchManager *batchManager = nullptr;
			};

			Shade(Data &data)
				: data(data)
			{}

			bool preExecute() override
			{
				if (!data.shadingPack->at(0).material)
				{
					packIdx = 1;
					if (data.shadingPack->size() == 1)
						return (false);
				}

				data.batchManager->mapBins();
				return (true);
			}

			void execute() override
			{
				thread_local std::array<LocalBin, 6> localBins;
				std::unique_ptr<Sampler> sampler = data.sampler->clone(1);

				std::vector<Sample> samples;

				while (true)
				{
					const uint32_t idx = packIdx.fetch_add(1);
					if (idx >= data.shadingPack->size())
						break;

					for (uint32_t i = data.shadingPack->at(idx).start; i <= data.shadingPack->at(idx).end; i++)
					{
						data.sampler->startPixel(Point2i(0, 0));
						if (data.batch->depths[i] >= data.maxDepth)
							continue;

						sh::BSDF bsdf = data.shadingPack->at(idx).material->sample(-data.batch->directions[i], data.interactions->at(i), sampler->get2D());
						if (!bsdf.Le.isBlack())
						{
							Sample s;
							s.color = data.batch->colors[i] * bsdf.Le;
							s.pixelID = data.batch->pixelIDs[i];
							samples.push_back(s);
						}

						if ((data.batch->colors[i] * bsdf.Li).isBlack() || bsdf.wi.length() == 0)
							continue;

						Ray r(data.interactions->at(i).p, bsdf.wi);

						const uint8_t vectorIndex = abs(bsdf.wi).maxDimension();
						const bool isNegative = std::signbit(bsdf.wi[vectorIndex]);
						const uint8_t index = vectorIndex * 2 + isNegative;
						if (localBins[index].feed(CompactRay(r, data.batch->colors[i] * bsdf.Li, data.batch->pixelIDs[i], data.batch->sampleIDs[i], data.batch->depths[i] + 1)))
							data.batchManager->feed(index, localBins[index]);
					}
				}

				for (uint32_t i = 0; i < localBins.size(); i++)
				{
					data.batchManager->feed(i, localBins[i]);
				}

				data.samplesGuard->lock();
				data.samples->insert(data.samples->end(), samples.begin(), samples.end());
				data.samplesGuard->unlock();
			}

			void postExecute() override
			{
				data.batchManager->unmapBins();
			}

		private:
			Data data;

			std::atomic<uint64_t> packIdx = 0;
			uint64_t zOrderMax = 0;
		};
	}
}