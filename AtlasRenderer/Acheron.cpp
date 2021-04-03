#include "Acheron.h"

#include <string>

#include "GenerateFirstRays.h"

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
	for (uint32_t i = 0; i < 16; i += sppStep)
	{
		uint32_t currentSpp = i + sppStep <= 16 ? sppStep : i + sppStep - 16;
		
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
	// extract batch
	threads.join();

	// sort rays

	// traverse

	// sort hitpoints

	// process hitpoints

	// launch one thread to addsample to the film
}