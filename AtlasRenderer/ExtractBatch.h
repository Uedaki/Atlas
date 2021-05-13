#pragma once

// Need to be above of the other include
#define NOMINMAX
#include <windows.h>

#include <FileApi.h>
#include <fstream>
#include <MemoryApi.h>
#include <string>

#include "Acheron.h"
#include "atlas/core/Batch.h"
#include "Bin.h"
#include "CompactRay.h"
#include "ThreadPool.h"

namespace atlas
{
	namespace task
	{
		class ExtractBatch : public ThreadedTask
		{
		public:
			static constexpr uint64_t zOrderStep = 516;

			struct Data
			{
				Batch *dst = nullptr;
				BatchManager *batchManager = nullptr;
			};

			ExtractBatch(Data &data)
				: data(data)
			{}

			bool preExecute() override;
			void execute() override;
			void postExecute() override;

			inline bool hasBatchToProcess() const
			{
				return  (size);
			}

		private:
			static constexpr uint32_t maxRayPerPass = 512;

			Data data;

			std::string filename;
			Bin::FileHandles handle;
			uint32_t size = 0;

			std::atomic<uint32_t> index = 0;
		};
	}
}