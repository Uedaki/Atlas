#pragma once

#include <cstdint>

#include "atlas/core/Logging.h"

namespace atlas
{
	namespace sh
	{
		template <typename T, size_t Size = 1>
		class ShadingOutput
		{
		public:
			static constexpr size_t byteSize = sizeof(T) * Size;

			ShadingOutput() = default;
			ShadingOutput(uint32_t &size)
				: pos(size)
			{
				size += sizeof(T) * Size;
			}

			void registerOutput(uint32_t &size)
			{
				pos = size;
				size += sizeof(T) * Size;
			}

			void set(std::vector<uint8_t> &data, const T &value, uint32_t idx = 0) const
			{
				CHECK(pos != -1);
				T *dst = (T *)&data[pos + idx];
				*dst = value;
			}

			uint32_t getPos(uint32_t index = 0) const
			{
				CHECK(pos != -1);
				return (pos + index * sizeof(T));
			}
		private:
			uint32_t pos = -1;
		};

		template <typename T>
		class ShadingInput
		{
		public:
			void connect(const ShadingOutput<T> &o, uint32_t index = 0)
			{
				pos = o.getPos(index);
			}

			const T &getValue(const std::vector<uint8_t> &data) const
			{
				CHECK(pos != -1);

				T *input = (T *)&data[pos];
				return (*input);
			}

		private:
			uint32_t pos = -1;
		};
	}
}