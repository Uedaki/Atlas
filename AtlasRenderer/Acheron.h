#pragma once

#include "Atlas/core/Camera.h"
#include "Atlas/core/Film.h"
#include "Atlas/core/Primitive.h"
#include "Atlas/core/Sampler.h"

#include "ThreadPool.h"

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
				std::string tmpFolder;
			} parameter;
		};

		Acheron(const Info &info);
		~Acheron();

		void render(const Camera &camera, const Primitive &scene);
		void renderIteration(const Camera &camera, const Primitive &scene, Film &film, uint32_t spp);
		void processBatches(const Primitive &scene, Film &film);

		// atlas::Film film(filmInfo);

		// atlas::PerspectiveCamera camera(worldToCam.inverse(), screen, 0.f, 1.f, 0, 10, 40, &film, nullptr);

		// atlas::BvhAccel bvh(primitives);

		// atlas::Sampler *sampler = atlas::StratifiedSampler::create(samplerInfo);


		// FilmInfo, bvh, sampler

		Sampler &getSampler()
		{
			return (*info.sampler);
		}

		const Point2i resolution;

		BatchManager manager;
	private:
		Info info;

		ThreadPool<8> threads;
	};
}