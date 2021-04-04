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
					for (uint32_t i = offset; i < end; i += 4)
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

		class S4ExtractBatch : public ThreadedTask
		{
		public:
			static constexpr uint64_t zOrderStep = 516;

			struct Data
			{
				S4Batch *dst = nullptr;
				BatchManager *batchManager = nullptr;
			};

			S4ExtractBatch(Data &data)
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
						return;
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
					for (uint32_t i = offset; i < end + 3; i += 4)
					{
						data.dst->origins[i] = S4Point3(handle.buffer[i].origin, handle.buffer[i + 1].origin, handle.buffer[i + 2].origin, handle.buffer[i + 3].origin);
						data.dst->directions[i] = S4Vec3(octDecode(handle.buffer[i].direction), octDecode(handle.buffer[i + 1].direction), octDecode(handle.buffer[i + 2].direction), octDecode(handle.buffer[i + 3].direction));
						data.dst->colors[i] = S4RgbSpectrum(toColor(handle.buffer[i].weight), toColor(handle.buffer[i + 1].weight), toColor(handle.buffer[i + 2].weight), toColor(handle.buffer[i + 3].weight));
						data.dst->pixelIDs[i] = S4Int(handle.buffer[i].pixelID, handle.buffer[i + 1].pixelID, handle.buffer[i + 2].pixelID, handle.buffer[i + 3].pixelID);
						data.dst->sampleIDs[i] = S4Int(handle.buffer[i].sampleID, handle.buffer[i + 1].pixelID, handle.buffer[i + 2].sampleID, handle.buffer[i + 3].sampleID);
						data.dst->depths[i] = S4Int(handle.buffer[i].depth, handle.buffer[i + 1].depth, handle.buffer[i + 2].depth, handle.buffer[i + 3].depth);
						data.dst->tNears[i] = S4Int(handle.buffer[i].tNear, handle.buffer[i + 1].tNear, handle.buffer[i + 2].tNear, handle.buffer[i + 3].tNear);
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
				return (size == 0);
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