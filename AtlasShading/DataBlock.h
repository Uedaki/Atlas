#pragma once

#include <cstdint>

namespace atlas
{
	class DataBlock
	{
	public:
		DataBlock() = default;
		DataBlock(uint32_t bSize)
			: bufferSize(bSize)
		{
			buffer = new uint8_t[bSize];
		}

		DataBlock(DataBlock &block, uint32_t start, uint32_t size)
			: bufferSize(size)
		{
			buffer = &block.buffer[start];
			bufferSize |= 0x40000000;
		}

		DataBlock(const DataBlock &block) = delete; // construction from another DataBlock is forbidden to prevent data ownership issue

		~DataBlock()
		{
			delete[] buffer;
		}

		template <typename T>
		void set(const T &val, uint32_t pos)
		{
			CHECK(pos != -1);
			T *ptr = (T *)&buffer[pos];
			*ptr = val;
		}

		template <typename T>
		const T &get(uint32_t pos) const
		{
			CHECK(pos != -1);
			T *val = (T *)&buffer[pos];
			return (*val);
		}

		uint32_t size() const
		{
			return (0x7FFFFFFF & bufferSize);
		}

		bool isSubBlock() const
		{
			return (0x40000000 & bufferSize);
		}

	private:
		uint8_t *buffer;
		uint32_t bufferSize;
	};
}