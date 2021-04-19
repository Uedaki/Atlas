#include "Acheron.h"

#include <string>
#include <filesystem>

#include "atlas/core/FilmIterator.h"
#include "atlas/core/Telemetry.h"

#include "GenerateFirstRays.h"
#include "ExtractBatch.h"
#include "SortRays.h"
#include "traceRays.h"
#include "Shade.h"
#include "iterationFilm.h"

#include "BSDF.h"
#include "Material.h"

using namespace atlas;

Acheron::Acheron(const Info &info)
	: samplePerPixel(info.samplePerPixel)
	, minLightBounce(info.minLightBounce)
	, maxLightBounce(info.maxLightBounce)
	, lightTreshold(info.lightTreshold)
	, sampler(*info.sampler)
	, localBinSize(info.localBinSize)
	, batchSize(info.batchSize)
	, temporaryFolder(info.temporaryFolder)
	, assetFolder(info.assetFolder)
{
	threads.init(info.threadCount);
}

Acheron::~Acheron()
{
	threads.shutdown();
}

void Acheron::render(const Camera &camera, const Primitive &scene, Film &film)
{
	TELEMETRY(achRender, "Acheron/render");

	//cleanTemporaryFolder(); need to make sure everything works perfectly to prevents erasing the wrong file
	batch.reserve(Bin::MaxSize);

	uint32_t sppStep = 1;
	FilmIterator iteration = film.createIterator();
	for (uint32_t i = 0; i < samplePerPixel; i += sppStep)
	{
		sppStep = std::min(sppStep * 2, 16u);
		uint32_t currentSpp = i + sppStep <= samplePerPixel ? sppStep : samplePerPixel - i;
		printf("i %d sppStep %d currentSpp %d samplePerPixel %d\n", i, sppStep, currentSpp, samplePerPixel);

		printf("new Iteration (%d spp)\n", currentSpp);
		iteration.start(currentSpp);
		renderIteration(camera, scene, film, iteration);
	}
	iteration.accumulate();
	batch.clear();
}

void Acheron::renderIteration(const Camera &camera, const Primitive &scene, const Film &film, FilmIterator &iteration)
{
	TELEMETRY(achIteration, "Acheron/render/iteration");

	{
		TELEMETRY(achGenFirstRays, "Acheron/render/iteration/generateFirstRays");
		task::GenerateFirstRays::Data data;
		data.resolution = film.resolution;
		data.spp = iteration.getSampleCount();
		data.camera = &camera;
		data.sampler = &sampler;
		data.batchManager = &batchManager;
		threads.execute<task::GenerateFirstRays>(data);
		threads.join();
	}

	processBatches(scene, iteration);
}

void Acheron::processBatches(const Primitive &scene, FilmIterator &iteration)
{
	while (true)
	{
		TELEMETRY(achProcessBatch, "acheron/render/processBatches");

		{
			Block<SurfaceInteraction> interactions(Bin::MaxSize);

			// extract batch
			{
				TELEMETRY(achExtractBatch, "acheron/render/processBatches/extractBatch");
#if 1
				task::ExtractBatch::Data data;
				data.dst = &batch;
				data.batchManager = &batchManager;
				threads.execute<task::ExtractBatch>(data);
#else
				task::S4ExtractBatch::Data data;
				data.dst = &s4Batch;
				data.batchManager = &manager;
				task::S4ExtractBatch task(data);
				threads.execute<task::S4ExtractBatch>(data);
#endif
				threads.join();
				if (batch.size() < 512)//!task->hasBatchToProcess())
				{
					processSmallBatches(scene, iteration);
					return;
				}
				printf("Batch %d\n", batch.size());
			}

			//// sort rays
			//{
			//	TELEMETRY(achSortRays, "acheron/render/processBatches/sortRays");
			//	task::SortRays::Data data;
			//	data.batch = &batch;
			//	threads.execute<task::SortRays>(data);
			//	threads.join();
			//}

			// trace rays
			{
				TELEMETRY(achTraceRays, "acheron/render/processBatches/traceRays");
				task::TraceRays::Data data;
				data.batch = &batch;
				data.scene = &scene;
				data.interactions = &interactions;
				threads.execute<task::TraceRays>(data);
				threads.join();
			}

			// sort hitpoints
			{
				TELEMETRY(achSortSi, "acheron/render/processBatches/sortSi");
				
				std::vector<uint32_t> ref(batch.size());
				for (uint32_t i = 0; i < batch.size(); i++)
					ref[i] = i;
		
				std::sort(ref.begin(), ref.end(), [&interactions](const uint32_t &s1, const uint32_t &s2)
					{
						return ((uint64_t)interactions[s1].material < (uint64_t)interactions[s2].material);
					});

				std::vector<SurfaceInteraction> si(interactions.size());
				for (uint32_t i = 0; i < batch.size(); i++)
				{
					if (i < ref[i])
					{
						batch.swap(i, ref[i]);
						std::swap(interactions[i], interactions[ref[i]]);
					}
					else if (ref[i] < i)
					{
						uint32_t ib = i;
						while (ref[ib] < i)
						{
							ib = ref[ib];
						}
						batch.swap(i, ref[ib]);
						std::swap(interactions[i], interactions[ref[ib]]);
					}
				}
			}

			// process hitpoints
			{
				TELEMETRY(achShading, "acheron/render/processBatches/shading");

				std::mutex samplesGuard;
				std::vector<Sample> samples;

				const uint32_t maxItPerPack = 512;
				std::vector<ShadingPack> shadingPack(1);
				shadingPack[0].start = 0;
				shadingPack[0].end = 0;
				shadingPack[0].material = interactions[0].material;
				for (uint32_t i = 1; i < batch.size(); i++)
				{
					if (shadingPack.back().material != interactions[i].material || (shadingPack.back().material && i - shadingPack.back().start >= maxItPerPack))
					{
						shadingPack.emplace_back();
						shadingPack.back().start = i;
						shadingPack.back().material = interactions[i].material;
						CHECK(interactions[i].material);
					}
					shadingPack.back().end = i;
				}

				task::Shade::Data data;
				data.startingIndex = shadingPack.front().material ? 0 : 1;
				data.maxDepth = maxLightBounce;
				data.batch = &batch;
				data.interactions = &interactions;
				data.shadingPack = &shadingPack;
				data.sampler = &sampler;
				data.samples = &samples;
				data.samplesGuard = &samplesGuard;
				data.batchManager = &batchManager;
				threads.execute<task::Shade>(data);

				if (!shadingPack.front().material)
				{
					for (uint32_t i = shadingPack.front().start; i < shadingPack.front().end; i++)
					{
						atlas::Vec3f unitDir = normalize(batch.directions[i]);
						auto t = 0.5 * (unitDir.y + 1.0);
						Spectrum color((1.0 - t) * atlas::Spectrum(1.f) + t * atlas::Spectrum(0.5, 0.7, 1.0));
						iteration.addSample(batch.pixelIDs[i], batch.colors[i] * color);
					}
				}

				threads.join();

				{
					for (Sample &sample : samples)
					{
						iteration.addSample(sample.pixelID, sample.color);
					}
				}
			}
		}
	}
}

Spectrum Acheron::getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, atlas::Sampler &sampler, int depth)
{
	if (depth >= maxLightBounce)
		return (atlas::Spectrum(0.f));

	atlas::SurfaceInteraction s;
	if (scene.intersect(r, s))
	{
		atlas::sh::BSDF bsdf = s.material->sample(-r.dir, s, sampler.get2D());
		if (bsdf.Li.isBlack())
			return (bsdf.Le);
		return (bsdf.Le + /* bsdf.pdf * */ bsdf.Li * getColorAlongRay(atlas::Ray(s.p, bsdf.wi), scene, sampler, depth + 1));
	}
	atlas::Vec3f unitDir = normalize(r.dir);
	auto t = 0.5 * (unitDir.y + 1.0);
	return ((1.0 - t) * atlas::Spectrum(1.f) + t * atlas::Spectrum(0.5, 0.7, 1.0));
}

void Acheron::processSmallBatches(const Primitive &scene, FilmIterator &iteration)
{
	while (batch.size())
	{
		printf("Small batch %d\n", batch.size());
		TELEMETRY(achrSmallBatches, "acheron/render/processBatches/smallBatches");
		for (uint32_t i = 0; i < batch.size(); i++)
		{
			sampler.startPixel(Point2i(0, 0));
			Ray r(batch.origins[i], batch.directions[i]);
			Spectrum color = batch.colors[i] * getColorAlongRay(r, scene, sampler, batch.depths[i]);
			iteration.addSample(batch.pixelIDs[i], color);
		}

		{
			task::ExtractBatch::Data data;
			data.dst = &batch;
			data.batchManager = &batchManager;
			threads.execute<task::ExtractBatch>(data);
			threads.join();
		}
	}
}

void Acheron::cleanTemporaryFolder()
{
	BOOL res;
	if (!CreateDirectoryA(temporaryFolder.c_str(), nullptr))
	{
		res = GetLastError();
		if (res == ERROR_ALREADY_EXISTS)
		{
			for (const auto &entry : std::filesystem::directory_iterator(temporaryFolder.c_str()))
			{
				std::filesystem::remove_all(entry.path());
			}
		}
		else if (res == ERROR_PATH_NOT_FOUND)
			CHECK_WIN_CALL(false);
	}
	SetCurrentDirectoryA(temporaryFolder.c_str());
}