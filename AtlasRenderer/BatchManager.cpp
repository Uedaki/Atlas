#include "BatchManager.h"

#include <fstream>

using namespace atlas;

BatchManager::BatchManager(uint32_t binSize)
	: binSize(binSize)
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
		Bin::open(bin, binSize);
	}
}

void BatchManager::mapBins()
{
	for (auto &bin : bins)
	{
		Bin::map(bin, binSize);
	}
}

void BatchManager::unmapBins()
{
	for (auto &bin : bins)
	{
		if (bin.currentFile.file)
			Bin::unmap(bin.currentFile, binSize);

		if (bin.prevFile.file)
			Bin::unmap(bin.prevFile, binSize);
	}
}

void BatchManager::feed(uint8_t idx, LocalBin &localBin)
{
	Bin &bin = bins[idx];

	uint32_t size = localBin.currentSize;
	uint32_t offset = bin.pos.fetch_add(size);
	if (offset + size >= binSize)
	{
		bin.guard.lock();

		if (offset >= binSize)
		{
			bin.guard.unlock();
			feed(idx, localBin);
			return;
		}

		uint32_t firstSize = binSize - offset;
		uint32_t secondSize = offset + size - binSize;

		memcpy(&bin.currentFile.buffer[offset], localBin.buffer.data(), firstSize * sizeof(CompactRay));
		postBatchAsActive(bin.filename);

		bin.filename = getNewBatchName();
		Bin::open(bin, binSize);

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