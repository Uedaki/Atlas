#include "pch.h"
#include "BinFile.h"

#include <FileApi.h>
#include <MemoryApi.h>

#include "Acheron.h"

const uint32_t BinFile::MaxSize = 32768;

//BinFile::BinFile(BatchManager &manager)
//	: manager(&manager)
//{}

BinFile::BinFile(const BinFile &bin)
{
	manager = bin.manager;
}

void BinFile::setManager(BatchManager &newManager)
{
	manager = &newManager;
}

void BinFile::map()
{
	open(filename);
}

void BinFile::unmap()
{
	if (prevFile.buffer)
		unmap(prevFile);
	unmap(currentFile);
}

void BinFile::unmap(FileHandles &handles)
{
	const size_t size = MaxSize * sizeof(CompactRay);
	FlushViewOfFile(handles.buffer, size);
	UnmapViewOfFile(handles.buffer);
	CloseHandle(handles.mapping);
	CloseHandle(handles.file);
	handles = { 0 };
}

void BinFile::open()
{
	open(manager->getNewBatchName());
}

void BinFile::open(const std::string &newFilename)
{
	if (prevFile.file)
		unmap(prevFile);
	prevFile = currentFile;

	filename = newFilename;
	const size_t size = sizeof(uint32_t) + MaxSize * sizeof(CompactRay);
	currentFile.file = CreateFileA(filename.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr);
	currentFile.mapping = CreateFileMappingA(currentFile.file, nullptr, PAGE_READWRITE, 0, size, nullptr);
	currentFile.buffer = (CompactRay*)MapViewOfFile(currentFile.mapping, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, size);
}

void BinFile::close()
{
	if (prevFile.file)
		unmap(prevFile);
	if (currentFile.file)
		unmap(currentFile);
	DeleteFileA(filename.c_str());
	filename.clear();
	pos = 0;
}

BinFile::~BinFile()
{
	close();
}

void BinFile::feed(const std::array<CompactRay, 256> &additionnalRays, uint32_t size)
{
	uint32_t offset = pos.fetch_add(size);

	if (offset >= MaxSize)
	{
		while (pos >= MaxSize)
		{
			std::unique_lock<std::mutex> lck(lock);
			dispatcher.wait(lck);
		}
		feed(additionnalRays, size);
	}
	else if (offset + size >= MaxSize)
	{
		uint32_t firstSize = MaxSize - offset;
		uint32_t secondSize = offset + size - MaxSize;

		memcpy(&currentFile.buffer[offset], additionnalRays.data(), firstSize);
		manager->postBatchAsActive(filename);

		open(manager->getNewBatchName());

		pos = secondSize;
		dispatcher.notify_all();

		if (secondSize > 0)
			memcpy(currentFile.buffer, &additionnalRays[firstSize], secondSize);
	}
	else
	{
		memcpy(&currentFile.buffer[offset], additionnalRays.data(), size * sizeof(CompactRay));
	}
}

void BinFile::purge(Batch &batch)
{
	batch.resize(pos);
	for (uint32_t i = 0; i < pos; i++)
	{
		NRay r;
		currentFile.buffer[i].extract(r);

		batch.origins[i] = r.origin;
		batch.directions[i] = r.dir;
		batch.colors[i] = r.weight;
		batch.pixelIDs[i] = r.pixelID;
		batch.sampleIDs[i] = r.sampleID;
		batch.depths[i] = r.depth;
		batch.tNears[i] = r.tNear;
	}

	close();
}

void BinFile::extract(const std::string &filename, Batch &batch)
{
	batch.resize(MaxSize);
	const size_t fileSize = MaxSize * sizeof(CompactRay);
	void *fileHandle = CreateFileA(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_DELETE_ON_CLOSE,
		nullptr);

	for (uint32_t i = 0; i < MaxSize; i++)
	{
		CompactRay cr;

		DWORD s = 0;
		OVERLAPPED ol = { 0 };
		ReadFile(fileHandle, &cr, sizeof(CompactRay), &s, &ol);

		NRay r;
		cr.extract(r);

		batch.origins[i] = r.origin;
		batch.directions[i] = r.dir;
		batch.colors[i] = r.weight;
		batch.pixelIDs[i] = r.pixelID;
		batch.sampleIDs[i] = r.sampleID;
		batch.depths[i] = r.depth;
		batch.tNears[i] = r.tNear;
	}
	CloseHandle(fileHandle);
}