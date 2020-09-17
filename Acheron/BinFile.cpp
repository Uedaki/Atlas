#include "pch.h"
#include "BinFile.h"

#include <FileApi.h>
#include <MemoryApi.h>

#include "Acheron.h"

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
	open(filename, true);
}

void BinFile::unmap()
{
	if (prevFile.buffer)
		unmap(prevFile);
	unmap(currentFile);
}

void BinFile::unmap(FileHandles &handles)
{
	const size_t size = BIN_SIZE * sizeof(CompactRay);
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

void BinFile::open(const std::string &newFilename, bool doesFileExist)
{
	if (prevFile.file)
		unmap(prevFile);
	prevFile = currentFile;

	uint32_t flags = doesFileExist ? OPEN_EXISTING : CREATE_ALWAYS;

	filename = newFilename;
	const size_t size = sizeof(uint32_t) + BIN_SIZE * sizeof(CompactRay);
	currentFile.file = CreateFileA(filename.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, flags, FILE_ATTRIBUTE_TEMPORARY, nullptr);
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

void BinFile::feed(const std::array<CompactRay, LOCAL_BIN_SIZE> &additionnalRays, uint32_t size)
{
	uint32_t offset = pos.fetch_add(size);

	if (offset >= BIN_SIZE)
	{
		while (pos >= BIN_SIZE)
		{
			std::unique_lock<std::mutex> lck(lock);
			dispatcher.wait(lck);
		}
		feed(additionnalRays, size);
	}
	else if (offset + size >= BIN_SIZE)
	{
		uint32_t firstSize = BIN_SIZE - offset;
		uint32_t secondSize = offset + size - BIN_SIZE;

		memcpy(&currentFile.buffer[offset], additionnalRays.data(), firstSize * sizeof(CompactRay));
		manager->postBatchAsActive(filename);

		open(manager->getNewBatchName());

		pos = secondSize;
		dispatcher.notify_all();

		if (secondSize > 0)
			memcpy(currentFile.buffer, &additionnalRays[firstSize], secondSize * sizeof(CompactRay));
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
	batch.resize(BIN_SIZE);
	const size_t fileSize = BIN_SIZE * sizeof(CompactRay);
	void *fileHandle = CreateFileA(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_DELETE_ON_CLOSE,
		nullptr);

	for (uint32_t i = 0; i < BIN_SIZE; i++)
	{
		CompactRay cr;

		DWORD s = 0;
		ReadFile(fileHandle, &cr, sizeof(CompactRay), &s, nullptr);

		NRay r;
		cr.extract(r);

		batch.origins[i] = r.origin;
		batch.directions[i] = r.dir;
		batch.colors[i] = r.weight;
		batch.pixelIDs[i] = r.pixelID;
		batch.sampleIDs[i] = r.sampleID;
		batch.depths[i] = r.depth;
		batch.tNears[i] = r.tNear;

		if (r.origin.x == 0)
			printf("0\n");
	}
	CloseHandle(fileHandle);
}