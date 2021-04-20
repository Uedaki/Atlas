#pragma once

#include <cstdint>
#include <string>
#include <mutex>

#include "Bin.h"
#include "LocalBin.h"

namespace atlas
{
	class BatchManager
	{
	public:
		BatchManager(uint32_t binSize);

		void openBins();
		void mapBins();
		void unmapBins();

		void feed(uint8_t idx, LocalBin &localBin);

		std::string getNewBatchName();

		void postBatchAsActive(const std::string &batchName);
		std::string popBatchName();

		Bin *getUncompledBatch();

		inline uint32_t getBinSize() const
		{
			return (binSize);
		}

	private:
		uint32_t binSize;

		std::mutex locker;
		std::atomic<uint32_t> nbrBatch = 0;
		std::array<Bin, 6> bins;
	};
}