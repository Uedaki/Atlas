#pragma once

#include <array>
#include <atomic>
#include <mutex>

#include "CompactRay.h"
#include "BatchManager.h"

# ifdef ACHERON_EXPORTS
#   define ACH  __declspec( dllexport )
# else
#   define ACH __declspec( dllimport )
# endif

struct Batch;

class BinFile
{
	const static uint32_t MaxSize;

	struct FileHandles
	{
		void *file;
		void *mapping;
		CompactRay *buffer;
	};

public:
	BinFile() = default;
	ACH BinFile(const BinFile &);
	ACH ~BinFile();

	ACH void setManager(BatchManager &manager);

	ACH void map();
	ACH void unmap();
	ACH void unmap(FileHandles &handles);
	
	ACH void open();
	ACH void open(const std::string &filename);
	ACH void close();

	ACH void feed(const std::array<CompactRay, 256> &rays, uint32_t size);

	void purge(Batch &batch);
	static void extract(const std::string &filename, Batch &batch);

	uint32_t getSize() const { return (pos); }

private:
	BatchManager *manager;
	std::string filename;

	FileHandles currentFile;
	FileHandles prevFile;

	std::atomic<uint32_t> pos;
	std::mutex lock;
	std::condition_variable dispatcher;
};