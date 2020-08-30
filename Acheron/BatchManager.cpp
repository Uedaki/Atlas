#include "pch.h"
#include "BatchManager.h"

#include <fstream>

BatchManager::BatchManager()
{
	std::ofstream file("stack.tmp", std::ios::trunc);
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
	std::ifstream file("stack.tmp", std::ios::in);
	if (file)
	{
		std::string newLine;
		std::getline(file, newLine);
		while (!newLine.empty())
		{
			if (!batchName.empty())
				content += batchName + "\n";
			batchName = newLine;
			std::getline(file, newLine);
		}
		file.close();

		std::ofstream newFile("stack.tmp", std::ios::out | std::ios::trunc);
		if (newFile)
		{
			newFile << content;
			newFile.close();
		}
	}
	return (batchName);
}