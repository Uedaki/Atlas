#pragma once

#include <functional>

#include "AtlasRendererLibHeader.h"
#include "Atlas/core/Camera.h"
#include "atlas/core/Primitive.h"
#include "Atlas/core/Film.h"
#include "Atlas/core/Light.h"

#include "ThreadPool.h"

namespace atlas
{
	// ref http://www.cs.uu.nl/docs/vakken/magr/2015-2016/slides/lecture%2008%20-%20variance%20reduction.pdf
	class NextEventEstimation
	{
	public:
		struct Info
		{
			uint32_t samplePerPixel = 16;
			uint32_t minLightBounce = 0; // unused for now
			uint32_t maxLightBounce = 9;
			Float tmin = 0;
			Float tmax = INFINITY;
			Float lightTreshold = (Float)0.01;

			Sampler *sampler;

			uint32_t threadCount = std::thread::hardware_concurrency() - 1;
			
			std::function<void(const atlas::Film &film)> endOfIterationCallback;
		};

		ATLAS_RENDERER NextEventEstimation(const Info &info);
		ATLAS_RENDERER ~NextEventEstimation();

		ATLAS_RENDERER void render(const Camera &camera, const Primitive &scene, const std::vector<std::shared_ptr<atlas::Light>> &lights, Film &film);

		ATLAS_RENDERER Spectrum getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, const std::vector<std::shared_ptr<atlas::Light>> &lights, Sampler &sampler, int depth);
		ATLAS_RENDERER Spectrum sampleLightSources(const Interaction &intr, const atlas::Primitive &scene, const std::vector<std::shared_ptr<atlas::Light>> &lights);

		inline void cancel()
		{
			isRunning = false;
		}

	private:
		class Job : public ThreadedTask
		{
		public:
			struct Info
			{
				NextEventEstimation *nee;
				const Camera *camera;
				const Primitive *scene;
				const std::vector<std::shared_ptr<atlas::Light>> *lights;
				Film *film;
				uint32_t spp;
			};

			Job(const Info &info)
				: nee(*info.nee)
				, camera(*info.camera)
				, scene(*info.scene)
				, lights(*info.lights)
				, film(*info.film)
				, spp(info.spp)
			{}

			ATLAS_RENDERER bool preExecute() override;
			ATLAS_RENDERER void execute() override;
			ATLAS_RENDERER void postExecute() override;

		private:
			NextEventEstimation &nee;
			const Camera &camera;
			const Primitive &scene;
			const std::vector<std::shared_ptr<atlas::Light>> &lights;
			Film &film;
			uint32_t spp;

			std::atomic<uint32_t> nextIndex = 0;
			std::vector<Bounds2i> ranges;
		};

		friend class Slave;

		std::atomic<bool> isRunning = false;

		uint32_t samplePerPixel = 16;
		uint32_t minLightBounce = 0; // unused for now
		uint32_t maxLightBounce = 9;
		Float tmin = 0;
		Float tmax = INFINITY;
		Float lightTreshold = (Float)0.01;

		Sampler &sampler;

		ThreadPool<8> threads;

		std::function<void(const atlas::Film &film)> endOfIterationCallback;
	};
}