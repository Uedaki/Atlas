#include "ThreadPool.h"
#include "Bin.h"

#include <iostream>


class TestTask : public atlas::ThreadedTask
{
	void preExecute() override
	{
		std::cout << "[FirstTask] Init" << std::endl;
	}

	void execute() override
	{
		uint32_t i = 1;
		for (i = 0; i < 1000000000; i++)
		{
		}
		std::cout << "[FirstTask]" << i << std::endl;
	}

	void postExecute() override
	{
		std::cout << "[FirstTask] Shutdown" << std::endl;
	}
};

class TestTask2 : public atlas::ThreadedTask
{
	void preExecute() override
	{
		std::cout << "[SecondTask] Init" << std::endl;
	}

	void execute() override
	{
		uint32_t i = 1;
		for (i = 0; i < 1000000000; i++)
		{
		}
		std::cout << "[SecondTask]" << i << std::endl;
	}

	void postExecute() override
	{
		std::cout << "[SecondTask] Shutdown" << std::endl;
	}
};

void testScheduler()
{
	atlas::ThreadPool<8> pool;
	std::cout << "[main] init" << std::endl;
	pool.init();

	for (uint32_t i = 0; i < 100000000; i++)
	{
	}

	auto start = std::chrono::steady_clock::now();

	TestTask t;
	TestTask t1;
	TestTask t2;
	TestTask t3;
	TestTask t4;
	TestTask t5;
	TestTask t6;
	TestTask t7;

	std::cout << "[main] execute" << std::endl;
	pool.execute(&t);
	//pool.execute(&t1);
	//pool.execute(&t2);
	//pool.execute(&t3);
	//pool.execute(&t4);
	//pool.execute(&t5);
	//pool.execute(&t6);
	//pool.execute(&t7);
	//pool.execute(&t2);

	pool.join();

	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;

	std::cout << "[main] shutdown" << std::endl;
	pool.shutdown();
}

#include <immintrin.h>

#include "../Atlas/includes/Atlas/core/Ray.h"
#include "../Atlas/includes/Atlas/core/Camera.h"

uint64_t posToZOrderIndex(uint32_t x, uint32_t y)
{
	return (_pdep_u32(x, 0x55555555) | _pdep_u32(y, 0xaaaaaaaa));
}

uint64_t zOrderIndexToPos(uint64_t index, uint32_t &x, uint32_t &y)
{
	x = static_cast<uint32_t>(_pext_u64(index, 0x5555555555555555));
	y = static_cast<uint32_t>(_pext_u64(index, 0xaaaaaaaaaaaaaaaa));
	return (index);
}


int main()
{
	std::atomic<uint64_t> zOrderPos = 0;

	uint64_t zOrderMax = 0;
	uint64_t zOrderStep = 516;

	thread_local std::array<LocalBin, 6> localBins;

	while (true)
	{
		const uint64_t offset = zOrderPos.fetch_add(zOrderStep);
		if (offset >= zOrderMax)
			break;

		for (uint64_t i = offset; i < offset + zOrderStep; i++)
		{
			uint32_t x;
			uint32_t y;
			zOrderIndexToPos(i, x, y);
			if (x < 700 && y < 500)
			{
				for (uint32_t s = 0; s < 4; s++)
				{
					atlas::Ray r;
					atlas::CameraSample cs = sampler->getCameraSample(pixel);
					camera.generateRay(cs, r);

					const uint8_t vectorIndex = maxIdx(abs(r.dir));
					const bool isNegative = std::signbit(r.dir[vectorIndex]);
					const uint8_t index = vectorIndex * 2 + isNegative;
					if (localBins[index].feed())
					{

					}
				}
			}
		}
	}
}