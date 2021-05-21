#include "Acheron.h"

#include <string>

#include "atlas/core/FilmIterator.h"
#include "atlas/core/Telemetry.h"

#include "GenerateFirstRays.h"
#include "ExtractBatch.h"
#include "SortRays.h"
#include "traceRays.h"
#include "ShadeInteractions.h"

#include "BSDF.h"
#include "Material.h"

using namespace atlas;

Acheron::Acheron(const Info &info)
	: samplePerPixel(info.samplePerPixel)
	, minLightBounce(info.minLightBounce)
	, maxLightBounce(info.maxLightBounce)
	, lightTreshold(info.lightTreshold)
	, batchManager(info.batchSize)
	, sampler(*info.sampler)
	, smallBatchTreshold(info.smallBatchTreshold)
	, localBinSize(info.localBinSize)
	, batchSize(info.batchSize)
	, temporaryDir(std::filesystem::absolute(info.temporaryFolder))
	, assetDir(std::filesystem::absolute(info.assetFolder))
	, endOfIterationCallback(info.endOfIterationCallback)
	, console(*info.console)
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

	prepareTemporaryDir();
	batch.reserve(batchSize);

	uint32_t sppStep = 1;
	FilmIterator iteration = film.createIterator();
	for (uint32_t i = 0; i < samplePerPixel; i += sppStep)
	{
		sppStep = std::min(sppStep * 2, 16u);
		uint32_t currentSpp = i + sppStep <= samplePerPixel ? sppStep : samplePerPixel - i;
		console << "i " << i << " sppStep " << sppStep << " currentSpp " << currentSpp << " samplePerPixel " << samplePerPixel << std::endl;

		iteration.start(currentSpp);
		renderIteration(camera, scene, film, iteration);
		if (endOfIterationCallback)
			endOfIterationCallback(film.resolution, iteration);
	}
	iteration.accumulate();
	restoreExecutionDir();
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
		data.localBinSize = localBinSize;
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

		Block<SurfaceInteraction> interactions(batchSize);

		{
			TELEMETRY(achExtractBatch, "acheron/render/processBatches/extractBatch");
			task::ExtractBatch::Data data;
			data.dst = &batch;
			data.batchManager = &batchManager;
			threads.execute<task::ExtractBatch>(data);
			threads.join();
			if (batch.size() <= smallBatchTreshold)
			{
				processSmallBatches(scene, iteration);
				return;
			}
			//console << "Batch " << batch.size() << std::endl;
		}

		//{
		//	TELEMETRY(achSortRays, "acheron/render/processBatches/sortRays");
		//	task::SortRays::Data data;
		//	data.batch = &batch;
		//	threads.execute<task::SortRays>(data);
		//	threads.join();
		//}

		{
			TELEMETRY(achTraceRays, "acheron/render/processBatches/traceRays");
			task::TraceRays::Data data;
			data.batch = &batch;
			data.scene = &scene;
			data.interactions = &interactions;
			threads.execute<task::TraceRays>(data);
			threads.join();
		}

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

			bool hasTask = shadingPack.front().material || shadingPack.size() > 1;
			if (hasTask)
			{
				task::ShadeInteractions::Data data;
				data.startingIndex = shadingPack.front().material ? 0 : 1;
				data.maxDepth = maxLightBounce;
				data.lightTreshold = lightTreshold;
				data.localBinSize = localBinSize;
				data.batch = &batch;
				data.interactions = &interactions;
				data.shadingPack = &shadingPack;
				data.sampler = &sampler;
				data.samples = &samples;
				data.samplesGuard = &samplesGuard;
				data.batchManager = &batchManager;
				threads.execute<task::ShadeInteractions>(data);
			}

			//if (shadingPack.front().material || shadingPack.size() > 1)
			//{
			//	batchManager.mapBins();

			//	thread_local std::array<LocalBin, 6> localBins =
			//	{ LocalBin(localBinSize), LocalBin(localBinSize), LocalBin(localBinSize),
			//	LocalBin(localBinSize), LocalBin(localBinSize), LocalBin(localBinSize) };
			//	for (uint32_t i = shadingPack.back().start; i <= shadingPack.back().end; i++)
			//	{
			//		sampler.startPixel(Point2i(0, 0));

			//		sh::BSDF bsdf = shadingPack.back().material->sample(-batch.directions[i], interactions.at(i), sampler.get2D());
			//		if (!bsdf.Le.isBlack())
			//		{
			//			iteration.addSample(batch.pixelIDs[i], batch.colors[i] * bsdf.Le);
			//		}

			//		if (luminance(batch.colors[i] * bsdf.Li) < lightTreshold || bsdf.wi.length() == 0)
			//		{
			//			printf("luminance too low\n");
			//			continue;
			//		}

			//		if (batch.depths[i] < maxLightBounce)
			//		{
			//			Ray r(interactions.at(i).p, bsdf.wi);
			//			const uint8_t vectorIndex = abs(bsdf.wi).maxDimension();
			//			const bool isNegative = std::signbit(bsdf.wi[vectorIndex]);
			//			const uint8_t index = vectorIndex * 2 + isNegative;
			//			if (localBins[index].feed(CompactRay(r, batch.colors[i] * bsdf.Li, batch.pixelIDs[i], batch.sampleIDs[i], batch.depths[i] + 1)))
			//				batchManager.feed(index, localBins[index]);
			//		}
			//		else
			//		{
			//			printf("max depth\n");
			//			iteration.addSample(batch.pixelIDs[i], batch.colors[i]);
			//		}
			//	}

			//	for (uint32_t i = 0; i < localBins.size(); i++)
			//	{
			//		batchManager.feed(i, localBins[i]);
			//	}
			//}

			if (!shadingPack.front().material)
			{
				for (uint32_t i = shadingPack.front().start; i <= shadingPack.front().end; i++)
				{
					atlas::Vec3f unitDir = normalize(batch.directions[i]);
					Float t = (Float)0.5 * (unitDir.y + (Float)1.0);
					Spectrum color(((Float)1.0 - t) * atlas::Spectrum(1.f) + t * atlas::Spectrum((Float)0.5, (Float)0.7, (Float)1.0));
					iteration.addSample(batch.pixelIDs[i], batch.colors[i] * color);
				}
			}

			if (hasTask)
			{
				threads.join();

				for (Sample &sample : samples)
				{
					iteration.addSample(sample.pixelID, sample.color);
				}
			}
		}
	}
}

Spectrum Acheron::getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, atlas::Sampler &sampler, uint32_t depth)
{
	if (depth >= maxLightBounce)
		return (atlas::Spectrum(0.f));

	atlas::SurfaceInteraction s;
	if (scene.intersect(r, s))
	{
		atlas::sh::BSDF bsdf = s.material->sample(-r.dir, s, sampler.get2D());
		if (luminance(bsdf.Li) < lightTreshold)
			return (bsdf.Le);
		return (bsdf.Le + /* bsdf.pdf * */ bsdf.Li * getColorAlongRay(atlas::Ray(s.p, bsdf.wi), scene, sampler, depth + 1));
	}
	atlas::Vec3f unitDir = normalize(r.dir);
	auto t = (Float)0.5 * (unitDir.y + (Float)1.0);
	return (((Float)1.0 - t) * atlas::Spectrum(1) + t * atlas::Spectrum((Float)0.5, (Float)0.7, (Float)1.0));
}

void Acheron::processSmallBatches(const Primitive &scene, FilmIterator &iteration)
{
	while (batch.size())
	{
		console << "Small batch " << batch.size() << std::endl;
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

void Acheron::prepareTemporaryDir()
{
	if (std::filesystem::exists(temporaryDir))
	{
		for (const auto &entry : std::filesystem::directory_iterator(temporaryDir))
		{
			if (entry.path().extension() == "tmp")
				std::filesystem::remove_all(entry.path());
		}
	}
	else
		std::filesystem::create_directory(temporaryDir);
	executionDir = std::filesystem::current_path();
	std::filesystem::current_path(temporaryDir);
}

void Acheron::restoreExecutionDir()
{
	std::filesystem::current_path(executionDir);
}