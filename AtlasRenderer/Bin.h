#pragma once

#include <array>
#include <cstdint>
#include <mutex>

#include "CompactRay.h"

namespace atlas
{
	struct Bin
	{
		struct FileHandles
		{
			void *file = nullptr;
			void *mapping = nullptr;
			CompactRay *buffer = nullptr;
		};

		std::string filename = "";

		FileHandles currentFile = {0};
		FileHandles prevFile = {0};

		std::atomic<uint32_t> pos = 0;
		std::mutex guard;
		std::condition_variable dispatcher;

		static Bin::FileHandles open(Bin &bin, uint32_t maxSize);
		static void map(Bin &bin, uint32_t maxSize);
		static void unmap(FileHandles &handle, uint32_t maxSize);

		static void reset(Bin &bin);
	};
}