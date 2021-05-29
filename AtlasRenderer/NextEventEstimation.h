#pragma once

#include "AtlasRendererLibHeader.h"
#include "Atlas/core/Camera.h"
#include "atlas/core/Primitive.h"
#include "Atlas/core/Film.h"

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
		};

		ATLAS_RENDERER NextEventEstimation(const Info &info);
		ATLAS_RENDERER ~NextEventEstimation();

		ATLAS_RENDERER void render(const Camera &camera, const Primitive &scene, Film &film);

		ATLAS_RENDERER Spectrum getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, int depth);

	private:
		uint32_t samplePerPixel = 16;
		uint32_t minLightBounce = 0; // unused for now
		uint32_t maxLightBounce = 9;
		Float tmin = 0;
		Float tmax = INFINITY;
		Float lightTreshold = (Float)0.01;

		Sampler &sampler;
	};
}