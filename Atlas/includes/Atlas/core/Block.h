#pragma once

#include <cstdint>

#include "Logging.h"

namespace atlas
{
	template <typename Type>
	class Block
	{
	public:
		Block()
			: buffer(nullptr)
			, bufferSize(0)
		{}
		Block(uint32_t bSize)
			: bufferSize(bSize)
		{
			buffer = new Type[bSize];
		}

		Block(Block &block, uint32_t start, uint32_t size)
			: bufferSize(size)
		{
			buffer = &block.buffer[start];
			bufferSize |= 0x40000000;
		}

		Block(const Block &block) = delete;

		~Block()
		{
			if (!isSubBlock() && buffer)
				delete[] buffer;
		}

		void resize(uint32_t size)
		{
			if (size != bufferSize)
			{
				if (!isSubBlock() && buffer)
					delete[] buffer;
				buffer = new Type[size];
				bufferSize = size;
			}
		}

		void clear()
		{
			if (!isSubBlock() && buffer)
				delete[] buffer;
			buffer = nullptr;
			bufferSize = 0;
		}

		inline Type &at(const uint32_t idx)
		{
			return (buffer[idx]);
		}

		inline const Type &at(const uint32_t idx) const
		{
			return (buffer[idx]);
		}

		inline Type &operator[](const size_t idx)
		{
			return (buffer[idx]);
		}

		inline const Type &operator[](const size_t idx) const
		{
			return (buffer[idx]);
		}

		uint32_t size() const
		{
			return (0x7FFFFFFF & bufferSize);
		}

		bool isSubBlock() const
		{
			return (0x40000000 & bufferSize);
		}

	protected:
		Type *buffer;
		uint32_t bufferSize;
	};

	class DataBlock : public Block<uint8_t>
	{
	public:
		DataBlock() = default;
		DataBlock(uint32_t bSize)
			: Block(bSize)
		{}

		DataBlock(DataBlock &block, uint32_t start, uint32_t size)
			: Block(block, start, size)
		{}

		DataBlock(const DataBlock &block) = delete; // construction from another DataBlock is forbidden to prevent data ownership issue

		~DataBlock()
		{}

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
	};
}