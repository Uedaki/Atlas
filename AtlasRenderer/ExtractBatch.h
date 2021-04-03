#pragma once

//		{
//		MULTIPLE_TIMED_SCOPE(tlyExtractBatch);
//		BinFile *bin = nullptr;
//		std::string batchName = batchManager.popBatchName();
//		if (!batchName.empty())
//		{
//#if 0	
//			BinFile::extract(batchName, batch);
//#else
//			ExtractBatchTask task(*this, batchName);
//			scheduler.dispatch(task);
//#endif
//		}
//#if 1
//		else if ((bin = bins.getBin()))
//		{
//			ExtractBatchTask task(*this, bin->getFilename(), bin->getSize());
//			scheduler.dispatch(task);
//			bin->close();
//		}
//		else
//			return;
//#else
//		else if (!bins.purge(batch))
//			return;
//#endif
//		}

#include "Acheron.h"
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
				Point2i resolution;
				uint32_t spp;

				const Camera *camera = nullptr;
				Sampler *sampler = nullptr;
				BatchManager *batchManager = nullptr;
			};

			ExtractBatch(Data &data)
				: data(data)
			{}

			void preExecute() override
			{
				bin.filename = data.batchManager->popBatchName();
				if (!bin.filename.empty())
				{
					Bin::map(bin);
				}
				else if ((bin = bins.getBin()))
				{
					ExtractBatchTask task(*this, bin->getFilename(), bin->getSize());
					scheduler.dispatch(task);
					bin->close();
				}
				else
					return;
			}

			void execute() override
			{
				while (true)
				{
					uint32_t offset = index.fetch_add(maxRayPerPass);
					if (offset >= bin.pos)
						break;

					uint32_t end = std::min(offset + maxRayPerPass, (uint32_t)size);
					for (uint32_t i = offset; i < end; i++)
					{
						NRay r;
						buffer[i].extract(r);

						owner.batch.origins[i] = r.origin;
						owner.batch.directions[i] = r.dir;
						owner.batch.colors[i] = r.weight;
						owner.batch.pixelIDs[i] = r.pixelID;
						owner.batch.sampleIDs[i] = r.sampleID;
						owner.batch.depths[i] = r.depth;
						owner.batch.tNears[i] = r.tNear;
					}
				}
			}

			void postExecute() override
			{
				
			}

		private:
			Data data;

			bool hasBatchToProcess;
			Bin bin;

			std::atomic<uint32_t> index = 0;
			uint32_t size = 0;
			const uint32_t maxRayPerPass = 512;
		};
	}
}