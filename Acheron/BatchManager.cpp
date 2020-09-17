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