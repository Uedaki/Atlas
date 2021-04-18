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
			Point2i resolution;
			Bounds2f region;
			uint32_t spp;

			Sampler *sampler;
			Filter *filter;

			uint32_t maxDepth = 9;

			struct
			{

				uint8_t threadCount = std::thread::hardware_concurrency() - 1;
				std::string tmpFolder = "./";
			} parameter;
		};

		ATLAS_RENDERER Acheron(const Info &info);
		ATLAS_RENDERER ~Acheron();

		ATLAS_RENDERER void render(const Camera &camera, const Primitive &scene);
		ATLAS_RENDERER void renderIteration(const Camera &camera, const Primitive &scene, IterationFilm &film, uint32_t spp);
		ATLAS_RENDERER void processBatches(const Primitive &scene, IterationFilm &film);
		
		ATLAS_RENDERER void cleanTemporaryFolder();

		ATLAS_RENDERER void processSmallBatches(const Primitive &scene, IterationFilm &film);
		ATLAS_RENDERER Spectrum getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, atlas::Sampler &sampler, int depth);

		// atlas::Film film(filmInfo);

		// atlas::PerspectiveCamera camera(worldToCam.inverse(), screen, 0.f, 1.f, 0, 10, 40, &film, nullptr);

		// atlas::BvhAccel bvh(primitives);

		// atlas::Sampler *sampler = atlas::StratifiedSampler::create(samplerInfo);


		// FilmInfo, bvh, sampler

		const Point2i resolution;

		BatchManager manager;
	private:
		Info info;
		Batch batch;
		ThreadPool<8> threads;
	};
}