#pragma once

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
				uint32_t spp = 0;

				uint32_t localBinSize = 512;

				const Camera *camera = nullptr;
				Sampler *sampler = nullptr;
				BatchManager *batchManager = nullptr;
			};

			GenerateFirstRays(Data &data)
				: data(data)
			{}

			bool preExecute() override;
			void execute() override;
			void postExecute() override;

		private:
			Data data;

			std::atomic<uint64_t> zOrderPos = 0;
			uint64_t zOrderMax = 0;
		};
	}
}