#include "pch.h"
#include "Acheron.h"

#include "Atlas/Bound.h"
#include "Atlas/Hitable.h"
#include "Atlas/Chronometer.h"
#include "Atlas/Tools.h"
#include "Atlas/rendering/Ray.h"
#include "Atlas/rendering/HitRecord.h"

#include <iostream>

#include "LocalBins.h"
#include "CompactRay.h"
#include "Telemetry.h"
#include "utils.h"

DEFINE_SECTOR(tlyBuildingAcc, "Acheron/Building acceleration structure");
DEFINE_SECTOR(tlyExecution, "Acheron/Execution");
DEFINE_SECTOR(tlyIteration, "Acheron/Iteration");
DEFINE_SECTOR(tlyBuildingBatches, "Acheron/Building batches");
DEFINE_SECTOR(tlyProcessRays, "Acheron/ProcessRays");
DEFINE_SECTOR(tlyExtractBatch, "Acheron/ProcessRays/Extract Batch");
DEFINE_SECTOR(tlySortRays, "Acheron/ProcessRays/Sort rays");

Acheron::Acheron(const atlas::Scene &s, uint32_t w, uint32_t h, uint32_t nbrS)
	: scene(s)
	, width(w), height(h), maxSample(nbrS)
	, bins(batchManager)
{
	SINGLE_TIMED_SCOPE(tlyBuildingAcc);

	output.resize(w * h);

	bool isFirst = true;
	atlas::Bound bound;
	std::vector<const atlas::Hitable *> elements;
	for (auto &shape : scene.getShapes())
	{
		elements.emplace_back(shape.get());
		if (isFirst)
		{
			bound = shape->getBound();
			isFirst = false;
		}
		else
			bound += shape->getBound();
	}
	accelerations.reserve(scene.getShapes().size() * 2);
	accelerations.emplace_back();
	accelerations.back().feed(elements, bound, accelerations);
}

Acheron::~Acheron()
{
	std::cout << "Stopping" << std::endl;

	for (uint32_t i = 0; i < 1000; i++)
		;

	isRendering = false;
	pushCommand(CommandType::STOP);

	for (auto &thread : threads)
	{
		if (thread.joinable())
			thread.join();
	}
}

void Acheron::launch(const atlas::Camera &c, uint32_t nbrThreads)
{
	isRendering = true;

	{
		SINGLE_TIMED_SCOPE(tlyExecution);
		camera = &c;
		zOrderMax = posToZOrderIndex(width, height);

		sleepingThread = nbrThreads;
		for (size_t i = 0; i < nbrThreads; i++)
		{
			threads.emplace_back([this]() { this->runSlave(); printf("Exit thread\n"); });
		}

		uint32_t step = 4;
		for (uint32_t i = 0; i < maxSample; i += step)
		{
			MULTIPLE_TIMED_SCOPE(tlyIteration);

			currentSampleTarget = i + step <= maxSample ? step : i + step - maxSample;
			runOneIteration();
			step = std::max(step * 2, 256u);
			break;
		}
	}

	PRINT_TELEMETRY_REPORT();
	
	pushCommand(CommandType::STOP);
}

#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;
#define M 7
#define NSTACK 50

void Acheron::quicksort()
{
	long i;
	long ir = batch.origins.size() - 1;
	long j;
	long k;
	long l = 0;
	
	uint32_t target;
	std::vector<glm::vec3> *order;

	std::vector<Pack> stack;
	int jstack = 0;
	float a, b, temp;

	if (batch.origins.size() > 4096)
	{
		target = 4096;
		order = &batch.origins;
	}
	else
	{
		target = 64;
		order = &batch.directions;
	}


	stack.resize(NSTACK);
	for (;;)
	{
		if (ir - l < target - 1)
		{
			uint32_t pivot = 0;
			while (pivot < l)
				pivot += target;

			if (ir - l < pivot - l)
			{
				if (target == 4096)
				{
					target = 64;
					order = &batch.directions;
					continue;
				}

				if (!jstack)
					return;

				jstack -= 1;
				l = stack[jstack].firstIndex;
				ir = l + stack[jstack].size - 1;
				target = stack[jstack].target;
				order = stack[jstack].order;

				continue;
			}
		}

		// Insertion sort when subarray small enough.
		if (ir - l < M)
		{
			uint8_t axis = pickLargestAxis(*order, l, ir);

			for (j = l + 1; j <= ir; j++)
			{
				a = (*order)[j][axis];
				NRay aValue = batch[j];
				for (i = j - 1; i >= l; i--)
				{
					if ((*order)[i][axis] <= a) break;
					batch.swap(i + 1, i);
				}
				batch.set(aValue, i + 1);
			}
			if (!jstack)
			{
				return;
			}
			jstack -= 1;
			l = stack[jstack].firstIndex;
			ir = l + stack[jstack].size - 1;
			target = stack[jstack].target;
			order = stack[jstack].order;
		}
		else
		{
			uint8_t axis = pickLargestAxis(*order, l, ir);

			// l = 1;
			// ir = n;
			k = (l + ir) >> 1; // find pivot point
			batch.swap(k, l + 1);
			if ((*order)[l][axis] > (*order)[ir][axis]) // if first point superior at last point swap point
				batch.swap(l, ir);
			if ((*order)[l + 1][axis] > (*order)[ir][axis]) // if first pont + 1 superior to last point, swap
				batch.swap(l + 1, ir);
			if ((*order)[l][axis] > (*order)[l + 1][axis]) //  if first point superior to first point + 1, swap
				batch.swap(l, l + 1);

			i = l + 1;
			j = ir;
			a = (*order)[l + 1][axis];
			NRay aValue = batch[l + 1];
			for (;;)
			{
				do
					i++;
				while ((*order)[i][axis] < a);

				do
					j--;
				while ((*order)[j][axis] > a);

				if (j < i)
					break;
				batch.swap(i, j);
			}

			batch.swap(l + 1, j); // Insert partitioning element in both arrays.
			batch.set(aValue, j);

			// Push pointers to larger subarray on stack, process smaller subarray immediately.
			if (jstack > NSTACK)
				printf("NSTACK too small in sort2.");
			if (ir - i + 1 >= j - l)
			{
				stack[jstack].size = ir + 1 - i;
				stack[jstack].firstIndex = i;
				stack[jstack].order = order;
				stack[jstack].target = target;
				ir = j - 1;
			}
			else
			{
				stack[jstack].size = j - l;
				stack[jstack].firstIndex = l;
				l = i;
			}
			stack[jstack].order = order;
			stack[jstack].target = target;
			jstack += 1;
		}
	}
}

void Acheron::runOneIteration()
{
	{
		MULTIPLE_TIMED_SCOPE(tlyBuildingBatches);

		// generate camera rays
		bins.open();
		pushCommand(CommandType::GEN_FIRST_RAYS);
		generateFirstRays();
		bins.unmap();
	}

	// processRays
	processRays();

	// update adaptive sampler
	// update cache points
	// save image files & checkpoint
}

void Acheron::processRays()
{
	MULTIPLE_TIMED_SCOPE(tlyProcessRays);

	while (true)
	{
		{
			MULTIPLE_TIMED_SCOPE(tlyExtractBatch);
			std::string batchName = batchManager.popBatchName();
			if (!batchName.empty())
				BinFile::extract(batchName, batch);
			else if (!bins.purge(batch))
				return;
		}

		{
			MULTIPLE_TIMED_SCOPE(tlySortRays);
#if 0
			packIndex = 0;
			packOffset = 1;
			packSize = 1;
			sortPack.resize(batch.origins.size() / 64);
			sortPack[0] = {&batch.origins, 0, (uint32_t)batch.origins.size(), 4096};
			sortNextRay();
			pushCommandForOne(CommandType::SORT_RAYS);
			while (sleepingThread != 0 && packIndex < packSize)
			{
				sortNextRay();
				pushCommand(CommandType::SORT_RAYS);
			}
			sortRays();
			sortPack.clear();
#else
			quicksort();
#endif
		}

		// traverse rays
		hitPoints.resize(batch.size());
		for (uint32_t i = 0; i < batch.origins.size(); i++)
		{
			atlas::rendering::Ray ray(batch.origins[i], batch.directions[i]);

			if (accelerations.front().hit(ray, tmin, tmax, hitPoints[i]))
				output.image[batch.pixelIDs[i]] += glm::vec3(1.f, 0.f, 0.f) * 0.25f;
			else
				output.image[batch.pixelIDs[i]] += glm::vec3(0.f, 0.f, 0.f);
		}

		// sort hit point

		// bins.map();
		// shade hit group
		// bins.unmap();
	}
}

void Acheron::runSlave()
{
	while (true)
	{
		sleepingThread -= 1;
		switch (command)
		{
		case (CommandType::GEN_FIRST_RAYS):
			generateFirstRays();
			break;
		case (CommandType::SORT_RAYS):
			sortRays();
			break;
		case (CommandType::STOP):
			return;
		default:
			break;
		}
		sleepingThread += 1;
		sleepUntilSignal();
	}
}

void Acheron::generateFirstRays()
{
	thread_local LocalBins localBins(bins);

	while (zOrderPos < zOrderMax)
	{
		const uint64_t offset = zOrderPos.fetch_add(zOrderStep);
		for (uint64_t i = offset; i < offset + zOrderStep; i++)
		{
			uint32_t x;
			uint32_t y;
			zOrderIndexToPos(i, x, y);
			if (x < width && y < height)
			{
				for (uint32_t s = 0; s < currentSampleTarget; s++)
				{
					const float u = (static_cast<float>(x) + atlas::Tools::rand()) / width;
					const float v = (static_cast<float>(y) + atlas::Tools::rand()) / height;

					const atlas::rendering::Ray ray = camera->getRay(u, v);
					CompactRay compactRay(ray, x + y * width, s, tmin);

						//atlas::rendering::HitRecord hit;

						//NRay r;
						//compactRay.extract(r);
						//atlas::rendering::Ray ray2(r.origin, octDecode(octEncode(glm::normalize(ray.dir))));

						//if (accelerations.front().hit(ray2, tmin, tmax, hit))
						//	output.image[compactRay.pixelID] += glm::vec3(1.f, 0.f, 0.f) * 0.25f;
						//else
						//	output.image[compactRay.pixelID] += glm::vec3(0.f, 0.f, 0.f);

					localBins.feed(ray, compactRay);
				}
			}
		}
	}
	localBins.flush();
}

uint8_t Acheron::pickLargestAxis(const std::vector<glm::vec3> &ar, uint32_t firstIndex, uint32_t lastIndex)
{
	glm::vec2 rangeX(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
	glm::vec2 rangeY(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
	glm::vec2 rangeZ(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
	for (uint32_t i = firstIndex; i <= lastIndex; i++)
	{
		rangeX[0] = std::min(rangeX[0], batch.origins[i].x);
		rangeX[1] = std::max(rangeX[1], batch.origins[i].x);

		rangeY[0] = std::min(rangeY[0], batch.origins[i].y);
		rangeY[1] = std::max(rangeY[1], batch.origins[i].y);

		rangeZ[0] = std::min(rangeZ[0], batch.origins[i].z);
		rangeZ[1] = std::max(rangeZ[1], batch.origins[i].z);
	}

	const float lengthX = rangeX[1] - rangeX[0];
	const float lengthY = rangeY[1] - rangeY[0];
	const float lengthZ = rangeZ[1] - rangeZ[0];

	return (lengthX < lengthY ? (lengthY < lengthZ ? 2 : 1) : (lengthX < lengthZ ? 2 : 0));
}

uint32_t Acheron::partition(std::vector<glm::vec3> &order, uint32_t major, uint32_t firstIndex, uint32_t lastIndex, uint32_t pivotIndex)
{
	const uint32_t pivotValue = order[pivotIndex][major];
	batch.swap(pivotIndex, lastIndex);

	uint32_t store = firstIndex;
	for (uint32_t idx = firstIndex; idx < lastIndex; idx++)
	{
		if (order[idx][major] <= pivotValue)
		{
			batch.swap(idx, store);
			store++;
		}
	}

	batch.swap(lastIndex, store);
	return (store);
}

void Acheron::sortRays()
{
	while (packIndex < packSize)
	{
		sortNextRay();
	}
}

void Acheron::sortNextRay()
{
	uint32_t index = packIndex.fetch_add(1);
	const Pack &pack = sortPack[index];

	uint32_t p = 0;
	uint32_t firstIndex = pack.firstIndex;
	uint32_t lastIndex = pack.firstIndex + pack.size > pack.order->size() ? pack.order->size() - 1 : firstIndex + pack.size - 1;
	const uint32_t pivotIndex = firstIndex + (lastIndex - firstIndex) * 0.5f;
	const uint32_t major = pickLargestAxis(*pack.order, firstIndex, lastIndex);
	while ((p = partition(*pack.order, major, firstIndex, lastIndex, pivotIndex)) != pivotIndex)
	{
		firstIndex = p < pivotIndex ? p + 1 : firstIndex;
		lastIndex = p > pivotIndex ? p - 1 : lastIndex;
	}

	if (pack.size * 0.5f > 4096)
	{
		uint32_t offset = packOffset.fetch_add(2);
		sortPack[offset] = { &batch.origins, pack.firstIndex, pack.size / 2, 4096};
		sortPack[offset + 1] = { &batch.origins, pack.firstIndex + pack.size / 2, pack.size / 2, 4096};
		packSize += 2;
	}
	else if (pack.size * 0.5f > 64)
	{
		uint32_t offset = packOffset.fetch_add(2);
		sortPack[offset] = { &batch.directions, pack.firstIndex, pack.size / 2, 64 };
		sortPack[offset + 1] = { &batch.directions, pack.firstIndex + pack.size / 2, pack.size / 2, 64 };
		packSize += 2;
	}
}

void Acheron::pushCommand(CommandType newCommand)
{
	command = newCommand;
	dispatcher.notify_all();
}

void Acheron::pushCommandForOne(CommandType newCommand)
{
	command = newCommand;
	dispatcher.notify_one();
}

void Acheron::sleepUntilSignal()
{
	std::unique_lock<std::mutex> lk(lock);
	dispatcher.wait(lk);
}