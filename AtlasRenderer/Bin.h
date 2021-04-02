#pragma once

#include <array>
#include <cstdint>
#include <mutex>

#include "CompactRay.h"

namespace atlas
{
	class BatchManager;

	class LocalBin
	{
	public:
		static constexpr uint32_t BinSize = 512;

		bool feed(const CompactRay &ray)
		{
			buffer[currentSize] = ray;
			currentSize += 1;
			return (currentSize == BinSize);
		}

		void reset()
		{
			currentSize = 0;
		}

		uint32_t getSize() const
		{
			return (currentSize);
		}

	private:
		friend class BatchManager;

		uint32_t currentSize = 0;
		std::array<CompactRay, BinSize> buffer;
	};

	struct Bin
	{
		static const uint32_t MaxSize = 32768;

		struct FileHandles
		{
			void *file;
			void *mapping;
			CompactRay *buffer;
		};

		std::string filename;

		FileHandles currentFile;
		FileHandles prevFile;

		std::atomic<uint32_t> pos;
		std::mutex guard;
		std::condition_variable dispatcher;

		static void open(Bin &bin);
		static void map(Bin &bin);
		static void unmap(FileHandles &handle);
	};

	class BatchManager
	{
	public:
		void openBins();
		void mapBins();
		void unmapBins();

		void feed(uint8_t idx, LocalBin &localBin);

		std::string getNewBatchName();

		void postBatchAsActive(const std::string &batchName);
		std::string popBatchName();

	private:
		std::mutex locker;
		std::atomic<uint32_t> nbrBatch = 0;

		std::array<Bin, 6> bins;
	};
}