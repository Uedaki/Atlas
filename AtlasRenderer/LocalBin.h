#pragma once

#include <vector>

#include "CompactRay.h"

namespace atlas
{
	class LocalBin
	{
	public:
		LocalBin(uint32_t size)
			: buffer(size)
		{
			DCHECK(pow(2, log2(size)) == size);
		}

		bool feed(const CompactRay &ray)
		{
			buffer[currentSize] = ray;
			currentSize += 1;
			return (currentSize == buffer.size());
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
		std::vector<CompactRay> buffer;
	};
}