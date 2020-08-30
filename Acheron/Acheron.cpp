#include "pch.h"
#include "Acheron.h"

#include "Atlas/Bound.h"
#include "Atlas/Hitable.h"
#include "Atlas/Chronometer.h"
#include "Atlas/Tools.h"
#include "Atlas/rendering/Ray.h"

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

Acheron::Acheron(const atlas::Scene &s, uint32_t w, uint32_t h, uint32_t nbrS)
	: scene(s)
	, width(w), height(h), maxSample(nbrS)
	, bins(batchManager)
{
	SINGLE_TIMED_SCOPE(tlyBuildingAcc);

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
	SINGLE_TIMED_SCOPE(tlyExecution);

	atlas::Chronometer chrono;
	chrono.start();

	camera = &c;
	zOrderMax = posToZOrderIndex(width, height);

	for (size_t i = 0; i < nbrThreads; i++)
	{
		threads.emplace_back([this]() { this->runSlave(); });
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

	PRINT_TELEMETRY_REPORT();
	
	pushCommand(CommandType::STOP);
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

		// sort rays

		// traverse rays

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
		switch (command)
		{
		case (CommandType::GEN_FIRST_RAYS):
			generateFirstRays();
			break;
		case (CommandType::STOP):
			return;
		default:
			break;
		}
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

					localBins.feed(ray, compactRay);
				}
			}
		}
	}
	localBins.flush();
}

void Acheron::pushCommand(CommandType newCommand)
{
	command = newCommand;
	dispatcher.notify_all();
}

void Acheron::sleepUntilSignal()
{
	std::unique_lock<std::mutex> lk(lock);
	dispatcher.wait(lk);
}