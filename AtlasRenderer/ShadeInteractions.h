#pragma once

#include <immintrin.h>

#include "Acheron.h"
#include "Bin.h"
#include "CompactRay.h"
#include "ThreadPool.h"

#include "BSDF.h"
#include "Material.h"
#include "LocalBin.h"

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
		class ShadeInteractions : public ThreadedTask
		{
		public:
			struct Data
			{
				uint32_t startingIndex = 0;
				uint32_t maxDepth = 16;

				uint32_t localBinSize = 512;

				Float tmin = 0;
				Float lightTreshold = (Float)0.01;

				std::vector<Sample> *samples = nullptr;
				std::mutex *samplesGuard = nullptr;

				Batch *batch = nullptr;
				Block<SurfaceInteraction> *interactions;

				std::vector<ShadingPack> *shadingPack;

				Sampler *sampler = nullptr;
				BatchManager *batchManager = nullptr;
			};

			ShadeInteractions(Data &data)
				: data(data)
			{}

			bool preExecute() override;
			void execute() override;
			void postExecute() override;

		private:
			Data data;

			std::atomic<uint32_t> packIdx = 0;
		};
	}
}