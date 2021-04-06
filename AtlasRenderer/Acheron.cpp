#include "Acheron.h"

#include <string>

#include "GenerateFirstRays.h"
#include "ExtractBatch.h"
#include "SortRays.h"

using namespace atlas;

Acheron::Acheron(const Info &info)
	: info(info)
	, resolution(info.resolution)
{
	threads.init(info.parameter.threadCount);
}

Acheron::~Acheron()
{
	threads.shutdown();
}

void Acheron::render(const Camera &camera, const Primitive &scene)
{
	uint32_t it = 0;
	uint32_t sppStep = 2;
	info.spp = 1;
	for (uint32_t i = 0; i < info.spp; i += sppStep)
	{
		uint32_t currentSpp = i + sppStep <= info.spp ? sppStep : i + sppStep - info.spp;
		
		FilmInfo filmInfo;
		filmInfo.resolution = info.resolution;
		filmInfo.cropWindow = info.region;
		filmInfo.filter = info.filter;
		filmInfo.filename += "iteration-" + std::to_string(it);
		atlas::Film film(filmInfo);

		renderIteration(camera, scene, film, currentSpp);
		sppStep = std::max(sppStep * 2, 16u);
	}
}

void Acheron::renderIteration(const Camera &camera, const Primitive &scene, Film &film, uint32_t spp)
{
	{
		task::GenerateFirstRays::Data data;
		data.resolution = resolution;
		data.spp = spp;
		data.camera = &camera;
		data.sampler = info.sampler;
		data.batchManager = &manager;
		task::GenerateFirstRays genFirstRays(data);
		threads.execute(&genFirstRays);
		threads.join();
	}

	processBatches(scene, film);
	film.writeImage();
}

void Acheron::processBatches(const Primitive &scene, Film &film)
{
	//while (true)
	{
		// extract batch
		{
#if 1
			task::ExtractBatch::Data data;
			data.dst = &batch;
			data.batchManager = &manager;
			task::ExtractBatch task(data);
#else
			task::S4ExtractBatch::Data data;
			data.dst = &s4Batch;
			data.batchManager = &manager;
			task::S4ExtractBatch task(data);
#endif
			threads.execute(&task);
			threads.join();
			if (!task.hasBatchToProcess())
				return;
		}
		
		// sort rays
		{
			task::SortRays::Data data;
			data.batch = &batch;
			task::SortRays task(data);
			threads.execute(&task);
			threads.join();
		}

		// traverse

		// sort hitpoints

		// process hitpoints

		// launch one thread to addsample to the film
	}
}