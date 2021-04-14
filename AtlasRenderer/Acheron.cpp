#include "Acheron.h"

#include <string>

#include "GenerateFirstRays.h"
#include "ExtractBatch.h"
#include "SortRays.h"
#include "traceRays.h"

#include "BSDF.h"
#include "Material.h"

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
	info.spp = 2;
	for (uint32_t i = 0; i < info.spp; i += sppStep)
	{
		uint32_t currentSpp = i + sppStep <= info.spp ? sppStep : i + sppStep - info.spp;
		
		FilmInfo filmInfo;
		filmInfo.resolution = info.resolution;
		filmInfo.cropWindow = info.region;
		filmInfo.filter = info.filter;
		filmInfo.filename += "iteration-" + std::to_string(it) + ".ppm";
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
		threads.execute<task::GenerateFirstRays>(data);
		threads.join();
	}

	processBatches(scene, film);
	film.writeImage();
}

void Acheron::processBatches(const Primitive &scene, Film &film)
{
	while (true)
	{
		{
			Batch batch(Bin::MaxSize);
			std::vector<SurfaceInteraction> interactions(Bin::MaxSize);

			// extract batch
			{
#if 1
				task::ExtractBatch::Data data;
				data.dst = &batch;
				data.batchManager = &manager;
				threads.execute<task::ExtractBatch>(data);
#else
				task::S4ExtractBatch::Data data;
				data.dst = &s4Batch;
				data.batchManager = &manager;
				task::S4ExtractBatch task(data);
				threads.execute<task::S4ExtractBatch>(data);
#endif
				threads.join();
				if (batch.size() == 0)//< 512)//!task->hasBatchToProcess())
				{
					processSmallBatches(batch, scene, film);
					return;
				}
				printf("Batch %d\n", batch.size());
			}

			// sort rays
			//{
			//	task::SortRays::Data data;
			//	data.batch = &batch;
			//	threads.execute<task::SortRays>(data);
			//	threads.join();
			//}

			// traverse
			{
				task::TraceRays::Data data;
				data.batch = &batch;
				data.scene = &scene;
				data.interactions = &interactions;
				threads.execute<task::TraceRays>(data);
				threads.join();
			}

			//std::sort(interactions.begin(), interactions.end(), [](const SurfaceInteraction &s1, const SurfaceInteraction &s2)
			//	{
			//		return ((uint64_t)s1.material < (uint64_t)s2.material);
			//	});

			manager.mapBins();
			std::array<LocalBin, 6> localBins;
			for (uint32_t i = 0; i < batch.size(); i++)
			{
				if (interactions[i].material)
				{
					film.addSample(batch.pixelIDs[i], Spectrum(1.f, 0, 0));
					continue;

					if (batch.depths[i] >= info.maxDepth)
					{
						film.addSample(batch.pixelIDs[i], Spectrum(0.f));
						continue;
					}

					sh::BSDF bsdf = interactions[i].material->sample(-batch.directions[i], interactions[i], info.sampler->get2D());
					if (bsdf.color.isBlack())
					{
						film.addSample(batch.pixelIDs[i], bsdf.color);
						continue;
					}

					if (bsdf.wi.length() == 0)
					{
						film.addSample(batch.pixelIDs[i], Spectrum(0.f));
						continue;
					}

					Ray r(interactions[i].p, bsdf.wi);

					const uint8_t vectorIndex = abs(bsdf.wi).maxDimension();
					const bool isNegative = std::signbit(bsdf.wi[vectorIndex]);
					const uint8_t index = vectorIndex * 2 + isNegative;
					if (localBins[index].feed(CompactRay(r, batch.colors[i] * bsdf.color, batch.pixelIDs[i], batch.sampleIDs[i], batch.depths[i] + 1)))
						manager.feed(index, localBins[index]);
				}
				else
				{
					atlas::Vec3f unitDir = normalize(batch.directions[i]);
					auto t = 0.5 * (unitDir.y + 1.0);
					Spectrum color((1.0 - t) * atlas::Spectrum(1.f) + t * atlas::Spectrum(0.5, 0.7, 1.0));
					film.addSample(batch.pixelIDs[i], batch.colors[i] * color);
				}
			}

			for (uint32_t i = 0; i < localBins.size(); i++)
			{
				manager.feed(i, localBins[i]);
			}

			manager.unmapBins();
		}

		// sort hitpoints

		// process hitpoints

		// launch one thread to addsample to the film
	}
}

Spectrum Acheron::getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, atlas::Sampler &sampler, int depth)
{
	if (depth >= info.maxDepth)
		return (atlas::Spectrum(0.f));

	atlas::SurfaceInteraction s;
	if (scene.intersect(r, s))
	{
		atlas::sh::BSDF bsdf = s.material->sample(-r.dir, s, sampler.get2D());
		if (bsdf.color.isBlack())
			return (bsdf.color);
		return (/* bsdf.pdf * */ bsdf.color * getColorAlongRay(atlas::Ray(s.p, bsdf.wi), scene, sampler, depth + 1));
	}
	atlas::Vec3f unitDir = normalize(r.dir);
	auto t = 0.5 * (unitDir.y + 1.0);
	return ((1.0 - t) * atlas::Spectrum(1.f) + t * atlas::Spectrum(0.5, 0.7, 1.0));
}

void Acheron::processSmallBatches(Batch &batch, const Primitive &scene, Film &film)
{
	while (batch.size())
	{
		printf("Small batch %d\n", batch.size());
		for (uint32_t i = 0; i < batch.size(); i++)
		{
			Ray r(batch.origins[i], batch.directions[i]);
			Spectrum color = batch.colors[i] * getColorAlongRay(r, scene, *info.sampler, batch.depths[i]);
			film.addSample(batch.pixelIDs[i], color);
		}

		{
			task::ExtractBatch::Data data;
			data.dst = &batch;
			data.batchManager = &manager;
			threads.execute<task::ExtractBatch>(data);
			threads.join();
		}
	}
}