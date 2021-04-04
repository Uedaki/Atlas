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

// Need to be above of the other include
#define NOMINMAX
#include <windows.h>

#include <FileApi.h>
#include <fstream>
#include <MemoryApi.h>
#include <string>

#include "Acheron.h"
#include "Batch.h"
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

			void preExecute() override
			{
				filename = data.batchManager->popBatchName();
				size = 0;
				
				if (filename.empty())
				{
					Bin *uncompletedBin;
					if ((uncompletedBin = data.batchManager->getUncompledBatch()))
					{
						filename = uncompletedBin->filename;
						size = uncompletedBin->pos;
						handle = uncompletedBin->currentFile;
					}
					else
						return ;
				}
				else
					size = Bin::MaxSize;

				const size_t byteSize = sizeof(uint32_t) + size * sizeof(CompactRay);

				uint32_t flags = OPEN_EXISTING;

				handle.file = CreateFileA(filename.c_str(), GENERIC_READ, 0, nullptr, flags, FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
				handle.mapping = CreateFileMappingA(handle.file, nullptr, PAGE_READONLY, 0, byteSize, nullptr);
				handle.buffer = (CompactRay *)MapViewOfFile(handle.mapping, FILE_MAP_READ, 0, 0, byteSize);
				data.dst->resize(size);
			}

			void execute() override
			{
				if (size == 0)
					return;

				while (true)
				{
					uint32_t offset = index.fetch_add(maxRayPerPass);
					if (offset >= size)
						break;

					uint32_t end = std::min(offset + maxRayPerPass, size);
					for (uint32_t i = offset; i < end; i++)
					{
						data.dst->origins[i] = handle.buffer[i].origin;
						data.dst->directions[i] = octDecode(handle.buffer[i].direction);
						data.dst->colors[i] = toColor(handle.buffer[i].weight);
						data.dst->pixelIDs[i] = handle.buffer[i].pixelID;
						data.dst->sampleIDs[i] = handle.buffer[i].sampleID;
						data.dst->depths[i] = handle.buffer[i].depth;
						data.dst->tNears[i] = handle.buffer[i].tNear;
					}
				}
			}

			void postExecute() override
			{
				if (size == 0)
					return;

				Bin::unmap(handle);
			}

			bool hasBatchToProcess()
			{
				return  (size == 0);
			}

		private:
			static constexpr uint32_t maxRayPerPass = 512;

			Data data;

			std::string filename;
			Bin::FileHandles handle;
			uint32_t size = Bin::MaxSize;

			std::atomic<uint32_t> index = 0;
		};
	}
}