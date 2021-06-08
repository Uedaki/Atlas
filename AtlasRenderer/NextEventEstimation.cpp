#include "NextEventEstimation.h"

#include "Atlas/core/Interaction.h"
#include "BSDF.h"
#include "Material.h"

atlas::NextEventEstimation::NextEventEstimation(const NextEventEstimation::Info &info)
	: samplePerPixel(info.samplePerPixel)
	, minLightBounce(info.minLightBounce)
	, maxLightBounce(info.maxLightBounce)
	, tmin(info.tmin), tmax(info.tmax)
	, lightTreshold(info.lightTreshold)
	, sampler(*info.sampler)
	, endOfIterationCallback(info.endOfIterationCallback)
{

}

atlas::NextEventEstimation::~NextEventEstimation()
{

}

atlas::Spectrum atlas::NextEventEstimation::sampleLightSources(const SurfaceInteraction &intr, const atlas::Primitive &scene, const std::vector<std::shared_ptr<atlas::Light>> &lights)
{
	Spectrum out(0);
	for (auto &light : lights)
	{
		Float pdf;
		Interaction pShape = dynamic_cast<DiffuseAreaLight *>(light.get())->shape->sample(intr, sampler.get2D(), pdf);
		//printf("p %f %f %f\n", pShape.p.x, pShape.p.y, pShape.p.z);
		if (dot(intr.shading.n, pShape.n) < 0 && dot(pShape.p - intr.p, intr.shading.n) > 0)
		{
			Ray r(pShape.p, normalize(intr.p - pShape.p), (intr.p - pShape.p).length() - 0.1);
			SurfaceInteraction s;
			//if (!scene.intersect(r, s))
				return dot(intr.n, -r.dir) * (100 / pow((intr.p - pShape.p).length(), 2)) * (1);
		}
		// 
		// 
		//Vec3f wi;
		//Float pdf;
		//VisibilityTester visibility;
		//Spectrum l = light->sampleLi(intr, sampler.get2D(), wi, pdf, visibility);
		//if (!l.isBlack() && visibility.unoccluded(scene))
		//{
		//	return (1);
		//}

		//Ray r;
		//SurfaceInteraction s;
		//Normal mLight;
		//Float pdf;
		//Float pdfd;
		//Spectrum l = light->sampleLe(sampler.get2D(), sampler.get2D(), 100, r, mLight, pdf, pdfd);
		//if (!l.isBlack() && !scene.intersect(r, s))
		//{
		//	return (1);
		//}
	}
	return (out);
}

atlas::Spectrum atlas::NextEventEstimation::getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, const std::vector<std::shared_ptr<atlas::Light>> &lights, int depth)
{
	if (depth <= 0)
		return (atlas::Spectrum(0.f));

	atlas::SurfaceInteraction s;
	if (scene.intersect(r, s))
	{
#if defined(SHADING)
		atlas::sh::BSDF bsdf = s.primitive->getMaterial()->sample(-r.dir, s, sampler.get2D());
		if (bsdf.Li.isBlack())
			return (bsdf.Le);

		Spectrum lightColor;
		if (luminance(lightColor = sampleLightSources(s, scene, lights)) > 0.5)
		{
			return (bsdf.Le + /* bsdf.pdf * */ bsdf.Li * lightColor);
		}
		else
			return (bsdf.Le + /* bsdf.pdf * */ bsdf.Li * getColorAlongRay(atlas::Ray(s.p + tmin * bsdf.wi, bsdf.wi, tmax), scene, lights, depth - 1));
#endif
	}
	else
	{
		return (BLACK);
		atlas::Vec3f unitDir = normalize(r.dir);
		Float t = (Float)0.5 * (unitDir.y + (Float)1.0);
		return (((Float)1.0 - t) * atlas::Spectrum(1) + t * atlas::Spectrum((Float)0.5, (Float)0.7, (Float)1.0));
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
		
		for (auto pixel : film.croppedPixelBounds)
		{
			sampler.startPixel(pixel);
			for (uint32_t s = 0; s < currentSpp; s++)
			{
				atlas::Ray r;
				atlas::CameraSample cs = sampler.getCameraSample(pixel);

				camera.generateRay(cs, r);
				r.tmax = tmax;

				atlas::Spectrum color = getColorAlongRay(r, scene, lights, maxLightBounce);

				film.addSample(pixel.x + pixel.y * film.resolution.x, color, 1);
				sampler.startNextSample();
			}
		}

		if (endOfIterationCallback)
			endOfIterationCallback(film);
	}

	if (!isRunning)
		printf("Rendering canceled\n");
	isRunning = false;
}