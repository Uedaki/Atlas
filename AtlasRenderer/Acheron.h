#pragma once

#include "Atlas/core/Camera.h"
#include "Atlas/core/Film.h"
#include "Atlas/core/Primitive.h"
#include "Atlas/core/Sampler.h"

#include "AtlasRendererLibHeader.h"
#include "ThreadPool.h"
#include "Bin.h"
#include "Batch.h"

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

			struct
			{
				uint8_t threadCount = std::thread::hardware_concurrency() - 1;
				std::string tmpFolder = "./";
			} parameter;
		};

		ATLAS_RENDERER Acheron(const Info &info);
		ATLAS_RENDERER ~Acheron();

		ATLAS_RENDERER void render(const Camera &camera, const Primitive &scene);
		ATLAS_RENDERER void renderIteration(const Camera &camera, const Primitive &scene, Film &film, uint32_t spp);
		ATLAS_RENDERER void processBatches(const Primitive &scene, Film &film);

		// atlas::Film film(filmInfo);

		// atlas::PerspectiveCamera camera(worldToCam.inverse(), screen, 0.f, 1.f, 0, 10, 40, &film, nullptr);

		// atlas::BvhAccel bvh(primitives);

		// atlas::Sampler *sampler = atlas::StratifiedSampler::create(samplerInfo);


		// FilmInfo, bvh, sampler

		const Point2i resolution;

		Batch batch;
		S4Batch s4Batch;
		BatchManager manager;
	private:
		Info info;

		ThreadPool<8> threads;
	};
}