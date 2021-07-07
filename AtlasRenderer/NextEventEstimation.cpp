#include "NextEventEstimation.h"

#include "Atlas/core/Interaction.h"
#include "BSDF.h"
#include "Material.h"

#include <iostream>

atlas::NextEventEstimation::NextEventEstimation(const NextEventEstimation::Info &info)
	: samplePerPixel(info.samplePerPixel)
	, minLightBounce(info.minLightBounce)
	, maxLightBounce(info.maxLightBounce)
	, tmin(info.tmin), tmax(info.tmax)
	, lightTreshold(info.lightTreshold)
	, useSkyBackground(info.useSkyBackground)
	, backgroundColor(info.backgroundColor)
	, sampler(*info.sampler)
	, endOfIterationCallback(info.endOfIterationCallback)
{
	threads.init(info.threadCount);
}

atlas::NextEventEstimation::~NextEventEstimation()
{
	threads.shutdown();
}

atlas::Spectrum atlas::NextEventEstimation::sampleLightSources(const SurfaceInteraction &intr, const atlas::Primitive &scene, const std::vector<std::shared_ptr<atlas::Light>> &lights)
{
	Float div = 0;
	Spectrum out(0);
	for (auto &light : lights)
	{
		Float pdf;
		Interaction pShape = dynamic_cast<DiffuseAreaLight *>(light.get())->shape->sample(intr, sampler.get2D(), pdf);
		Vec3f toLight = pShape.p - intr.p;
		Float distanceSquared = toLight.lengthSquared();
		toLight = normalize(toLight);

		if (dot(toLight, intr.n) >= 0)
		{
			Float lightArea = dynamic_cast<DiffuseAreaLight *>(light.get())->shape->area();
			Float lightCosine = std::abs(dot(pShape.n, -toLight));
			if (lightCosine < 0.0000001)
				continue;

			Float pdf = (lightCosine * lightArea) / distanceSquared;
			Ray r(intr.p + tmin * intr.n, toLight, intr.time);
			SurfaceInteraction s;
			if (!scene.intersect(r, s) || s.primitive->getAreaLight())
			{
				BSDF bsdf = intr.primitive->getMaterial()->evaluate(intr.wo, r.dir, s);
				out += bsdf.scatteringPdf * bsdf.Li * (dynamic_cast<DiffuseAreaLight *>(light.get())->lEmit / distanceSquared) / bsdf.pdf;
				div++;
			}
		}
	}
	return (div != 0 ? out / div : out);
}

atlas::Spectrum atlas::NextEventEstimation::getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, const std::vector<std::shared_ptr<atlas::Light>> &lights, Sampler &sampler, int depth)
{
	if (depth <= 0)
		return (atlas::Spectrum(0.f));

	atlas::SurfaceInteraction s;
	if (scene.intersect(r, s))
	{
		if (s.primitive->getAreaLight())
			return (dynamic_cast<const DiffuseAreaLight*>(s.primitive->getAreaLight())->lEmit);

		BSDFSample bsdf = s.primitive->getMaterial()->sample(-r.dir, s, sampler.get2D());
		if (luminance(bsdf.Li) < lightTreshold)
			return (bsdf.Le);

		Spectrum ld = sampleLightSources(s, scene, lights);

		Ray r(s.p + tmin * s.n, bsdf.wi, tmax);
		Spectrum Li = getColorAlongRay(r, scene, lights, sampler, depth - 1);
		
		//printf("color %f %f %f sPdf %f pdf %f\n", bsdf.Li.r, bsdf.Li.g, bsdf.Li.b, bsdf.scatteringPdf, bsdf.pdf);

		return (bsdf.Le + 0.5 * (bsdf.scatteringPdf * bsdf.Li * Li / bsdf.pdf + ld));		
		
		//return (bsdf.Li * ei);// (PI * (Float)2 * bsdf.Li * ei + ld);
	}
	else if (useSkyBackground)
	{
		atlas::Vec3f unitDir = normalize(r.dir);
		Float t = (Float)0.5 * (unitDir.y + (Float)1.0);
		return ((Float)1.0 - t) * atlas::Spectrum(1) + t * atlas::Spectrum((Float)0.5, (Float)0.7, (Float)1.0);
	}
	else
	{
		return (backgroundColor);
	}
}

void atlas::NextEventEstimation::render(const Camera &camera, const Primitive &scene, const std::vector<std::shared_ptr<atlas::Light>> &lights, Film &film)
{
	isRunning = true;
	uint32_t sppStep = 1;
	for (uint32_t i = 0; isRunning && i < samplePerPixel; i += sppStep)
	{
		sppStep = std::min(sppStep * 2, 16u);
		uint32_t currentSpp = i + sppStep <= samplePerPixel ? sppStep : samplePerPixel - i;
		
#if 1
		Job::Info info;
		info.nee = this;
		info.camera = &camera;
		info.scene = &scene;
		info.lights = &lights;
		info.film = &film;
		info.spp = currentSpp;
		threads.execute<Job>(info);
		threads.join();
#else
		for (auto pixel : film.croppedPixelBounds)
		{
			sampler.startPixel(pixel);
			for (uint32_t s = 0; s < currentSpp; s++)
			{
				atlas::Ray r;
				atlas::CameraSample cs = sampler.getCameraSample(pixel);

				camera.generateRay(cs, r);
				r.tmax = tmax;

				atlas::Spectrum color = getColorAlongRay(r, scene, lights, sampler, maxLightBounce);

				film.addSample(pixel.x + pixel.y * film.resolution.x, color, 1);
				sampler.startNextSample();
			}
		}
#endif
		if (endOfIterationCallback)
			endOfIterationCallback(film);
	}

	if (!isRunning)
		printf("Rendering canceled\n");
	isRunning = false;
}

bool atlas::NextEventEstimation::Job::preExecute()
{
	for (int32_t j = 0; j < film.resolution.y; j += 64)
	{
		for (int32_t i = 0; i < film.resolution.x; i += 64)
		{
			Point2i pMin(i, j);
			Point2i pMax = min(Point2i(i + 64, j + 64), film.resolution);
			ranges.emplace_back(pMin, pMax);
		}
	}
	return (true);
}

void atlas::NextEventEstimation::Job::execute()
{
	std::hash<std::thread::id> hasher;
	std::unique_ptr<Sampler> sampler = nee.sampler.clone((int)hasher(std::this_thread::get_id()) * (spp + 1));

	while (nee.isRunning)
	{
		uint32_t index = nextIndex.fetch_add(1);
		if (index >= ranges.size())
			return;

		for (auto pixel : ranges[index])
		{
			sampler->startPixel(pixel);
			for (uint32_t s = 0; s < spp; s++)
			{
				atlas::Ray r;
				atlas::CameraSample cs = sampler->getCameraSample(pixel);

				camera.generateRay(cs, r);
				r.tmax = nee.tmax;

				atlas::Spectrum color = nee.getColorAlongRay(r, scene, lights, *sampler, nee.maxLightBounce);

				film.addSample(pixel.x + pixel.y * film.resolution.x, color, 1);
				auto c = film.getPixel(pixel.x + pixel.y * film.resolution.x);
				sampler->startNextSample();
			}
		}
	}
}

void atlas::NextEventEstimation::Job::postExecute()
{

}