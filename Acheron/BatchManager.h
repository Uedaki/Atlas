#pragma once

#include <mutex>
#include <atomic>
#include <string>

# ifdef ACHERON_EXPORTS
#   define ACH  __declspec( dllexport )
# else
#   define ACH __declspec( dllimport )
# endif

class BatchManager
{
public:
	ACH BatchManager();

	ACH std::string getNewBatchName();
	ACH void postBatchAsActive(const std::string &batchName);

	ACH std::string popBatchName();

private:
	std::mutex locker;
	std::atomic<uint32_t> nbrBatch = 0;
};