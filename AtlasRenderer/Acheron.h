#pragma once

#include "Atlas/core/Camera.h"
#include "Atlas/core/Film.h"
#include "Atlas/core/Primitive.h"
#include "Atlas/core/Sampler.h"

#include "AtlasRendererLibHeader.h"
#include "ThreadPool.h"
#include "Bin.h"
#include "atlas/core/Batch.h"
#include "IterationFilm.h"

namespace atlas
{
	class Acheron
	{
	public:

		struct Info
		{
			uint32_t samplePerPixel = 16;
			uint32_t minLightBounce = 0; // unused for now
			uint32_t maxLightBounce = 9;
			Float lightTreshold  = 0.01;

			Sampler *sampler;

			std::string temporaryFolder = "./";
			std::string assetFolder = "./";

			uint32_t localBinSize = 512;
			uint32_t batchSize = 65536;

			uint32_t threadCount = std::thread::hardware_concurrency() - 1;
		};

		ATLAS_RENDERER Acheron(const Info &info);
		ATLAS_RENDERER ~Acheron();

		ATLAS_RENDERER void render(const Camera &camera, const Primitive &scene, Film &film);
		ATLAS_RENDERER void renderIteration(const Camera &camera, const Primitive &scene, const Film &film, FilmIterator &iteration);
		ATLAS_RENDERER void processBatches(const Primitive &scene, FilmIterator &iteration);
		
		ATLAS_RENDERER void cleanTemporaryFolder();

		ATLAS_RENDERER void processSmallBatches(const Primitive &scene, FilmIterator &iteration);
		ATLAS_RENDERER Spectrum getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, atlas::Sampler &sampler, int depth);

		// atlas::Film film(filmInfo);

		// atlas::PerspectiveCamera camera(worldToCam.inverse(), screen, 0.f, 1.f, 0, 10, 40, &film, nullptr);

		// atlas::BvhAccel bvh(primitives);

		// atlas::Sampler *sampler = atlas::StratifiedSampler::create(samplerInfo);


		// FilmInfo, bvh, sampler

	private:
		uint32_t samplePerPixel = 16;
		uint32_t minLightBounce = 0; // unused for now
		uint32_t maxLightBounce = 9;
		Float lightTreshold = 0.01;

		BatchManager batchManager;
		Batch batch;

		Sampler &sampler;

		ThreadPool<8> threads;

		uint32_t localBinSize;
		uint32_t batchSize;

		const std::string temporaryFolder = "./";
		const std::string assetFolder = "./";
	};
}