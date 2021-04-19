#include "Bin.h"

// Need to be above of the other include
#include <windows.h>

#include <FileApi.h>
#include <fstream>
#include <MemoryApi.h>
#include <string>

//#include "Atlas/core/Telemetry.h"

using namespace atlas;

void Bin::open(Bin &bin)
{
	if (bin.prevFile.file)
		Bin::unmap(bin.prevFile);
	bin.prevFile = bin.currentFile;

	uint32_t flags = CREATE_ALWAYS;
	const size_t size = sizeof(uint32_t) + Bin::MaxSize * sizeof(CompactRay);
	bin.currentFile.file = CreateFileA(bin.filename.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, flags, FILE_ATTRIBUTE_TEMPORARY, nullptr);
	CHECK_WIN_CALL(bin.currentFile.file != INVALID_HANDLE_VALUE);
	bin.currentFile.mapping = CreateFileMappingA(bin.currentFile.file, nullptr, PAGE_READWRITE, 0, size, nullptr);
	CHECK_WIN_CALL(bin.currentFile.mapping != INVALID_HANDLE_VALUE);
	bin.currentFile.buffer = (CompactRay *)MapViewOfFile(bin.currentFile.mapping, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, size);
	CHECK_WIN_CALL(bin.currentFile.buffer != nullptr);

	bin.pos = 0;
}

void Bin::map(Bin &bin)
{
	if (bin.prevFile.file)
		Bin::unmap(bin.prevFile);
	bin.prevFile = bin.currentFile;

	uint32_t flags = OPEN_EXISTING;
	const size_t size = sizeof(uint32_t) + Bin::MaxSize * sizeof(CompactRay);
	bin.currentFile.file = CreateFileA(bin.filename.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, flags, FILE_ATTRIBUTE_TEMPORARY, nullptr);
	CHECK_WIN_CALL(bin.currentFile.file != INVALID_HANDLE_VALUE);
	bin.currentFile.mapping = CreateFileMappingA(bin.currentFile.file, nullptr, PAGE_READWRITE, 0, size, nullptr);
	CHECK_WIN_CALL(bin.currentFile.mapping != INVALID_HANDLE_VALUE);
	bin.currentFile.buffer = (CompactRay *)MapViewOfFile(bin.currentFile.mapping, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, size);
	CHECK_WIN_CALL(bin.currentFile.buffer != nullptr);
}

void Bin::unmap(Bin::FileHandles &handles)
{
	const size_t size = Bin::MaxSize * sizeof(CompactRay);
	FlushViewOfFile(handles.buffer, size);
	UnmapViewOfFile(handles.buffer);
	CloseHandle(handles.mapping);
	CloseHandle(handles.file);
	handles = { 0 };
}

void Bin::reset(Bin &bin)
{

}

BatchManager::BatchManager()
{
	std::ifstream file("stack.tmp", std::ios::trunc);
	if (file)
		file.close();
}

void BatchManager::openBins()
{
	for (auto &bin : bins)
	{
		bin.filename = getNewBatchName();
		Bin::open(bin);
	}
}

void BatchManager::mapBins()
{
	for (auto &bin : bins)
	{
		Bin::map(bin);
	}
}

void BatchManager::unmapBins()
{
	for (auto &bin : bins)
	{
		if (bin.currentFile.file)
			Bin::unmap(bin.currentFile);

		if (bin.prevFile.file)
			Bin::unmap(bin.prevFile);
	}
}

void BatchManager::feed(uint8_t idx, LocalBin &localBin)
{
	//TELEMETRY(tt, "tttt");
	Bin &bin = bins[idx];

	uint32_t size = localBin.currentSize;
	uint32_t offset = bin.pos.fetch_add(size);
	if (offset + size >= Bin::MaxSize)
	{
		bin.guard.lock();

		if (offset >= Bin::MaxSize)
		{
			bin.guard.unlock();
			feed(idx, localBin);
			return;
		}

		uint32_t firstSize = Bin::MaxSize - offset;
		uint32_t secondSize = offset + size - Bin::MaxSize;

		memcpy(&bin.currentFile.buffer[offset], localBin.buffer.data(), firstSize * sizeof(CompactRay));
		postBatchAsActive(bin.filename);

		bin.filename = getNewBatchName();
		Bin::open(bin);

		bin.pos = secondSize;
		bin.guard.unlock();

		if (secondSize > 0)
			memcpy(bin.currentFile.buffer, &localBin.buffer[firstSize], secondSize * sizeof(CompactRay));
	}
	else
	{
		memcpy(&bin.currentFile.buffer[offset], localBin.buffer.data(), size * sizeof(CompactRay));
	}

	localBin.reset();
}

std::string BatchManager::getNewBatchName()
{
	const uint32_t idx = nbrBatch;
	nbrBatch++;
	return ("batch-" + std::to_string(idx) + ".tmp");
}

void BatchManager::postBatchAsActive(const std::string &batchName)
{
	locker.lock();
	std::ofstream file("stack.tmp", std::ios::app);
	if (file)
	{
		file << batchName << "\n";
		file.close();
	}
	locker.unlock();
}

std::string BatchManager::popBatchName()
{
	std::string content;
	std::string batchName;
	std::ifstream file("stack.tmp");

	if (file)
	{
		if (file.eof())
			return ("");

		std::string newLine;
		std::getline(file, newLine);
		while (!file.eof() && !newLine.empty())
		{
			if (!batchName.empty())
				content += batchName + "\n";
			batchName = newLine;
			std::getline(file, newLine);
		}
		file.close();

		std::ofstream newFile("stack.tmp", std::ios::trunc);
		if (newFile)
		{
			newFile << content;
			newFile.close();
		}
	}
	return (batchName);
}

Bin *BatchManager::getUncompledBatch()
{
	uint32_t fullestBin = 0;
	for (uint8_t i = 1; i < 6; i++)
	{
		if (bins[i].pos > bins[fullestBin].pos)
			fullestBin = i;
	}
	if (bins[fullestBin].pos > 0)
	{
		return (&bins[fullestBin]);
	}
	else
		return (nullptr);
}