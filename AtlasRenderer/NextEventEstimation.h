#pragma once

#include <functional>

#include "AtlasRendererLibHeader.h"
#include "Atlas/core/Camera.h"
#include "atlas/core/Primitive.h"
#include "Atlas/core/Film.h"
#include "Atlas/core/Light.h"

namespace atlas
{
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

			std::function<void(const atlas::Film &film)> endOfIterationCallback;
		};

		ATLAS_RENDERER NextEventEstimation(const Info &info);
		ATLAS_RENDERER ~NextEventEstimation();

		ATLAS_RENDERER void render(const Camera &camera, const Primitive &scene, const std::vector<std::shared_ptr<atlas::Light>> &lights, Film &film);

		ATLAS_RENDERER Spectrum getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, const std::vector<std::shared_ptr<atlas::Light>> &lights, int depth);

		inline void cancel()
		{
			isRunning = false;
		}

	private:
		std::atomic<bool> isRunning = false;

		uint32_t samplePerPixel = 16;
		uint32_t minLightBounce = 0; // unused for now
		uint32_t maxLightBounce = 9;
		Float tmin = 0;
		Float tmax = INFINITY;
		Float lightTreshold = (Float)0.01;

		Sampler &sampler;

		std::function<void(const atlas::Film &film)> endOfIterationCallback;
	};
}