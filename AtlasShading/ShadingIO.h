#pragma once

#include <cstdint>

#include "atlas/core/Logging.h"
#include "DataBlock.h"

namespace atlas
{
	namespace sh
	{
		template <typename T>
		class ShadingOutput
		{
		public:
			ShadingOutput() = default;
			ShadingOutput(uint32_t &size)
				: pos(size)
			{
				size += sizeof(T);
			}

			void registerOutput(uint32_t &size)
			{
				pos = size;
				size += sizeof(T);
			}

			void set(DataBlock &block, const T &value) const
			{
				block.set(value, pos);
			}

			uint32_t getPos() const
			{
				CHECK(pos != -1);
				return (pos);
			}
		private:
			uint32_t pos = -1;
		};

		template <typename T>
		class ShadingInput
		{
		public:
			void bind(const ShadingOutput<T> &o)
			{
				pos = o.getPos();
			}

			const T &get(const DataBlock &block) const
			{
				return (block.get<T>(pos));
			}

		private:
			uint32_t pos = -1;
		};
	}
}