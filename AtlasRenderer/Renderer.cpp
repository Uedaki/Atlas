#include "Renderer.h"

#include "BSDF.h"

atlas::Renderer::Renderer(const Renderer::Info &info)
	: samplePerPixel(info.samplePerPixel)
	, minLightBounce(info.minLightBounce)
	, maxLightBounce(info.maxLightBounce)
	, tmin(info.tmin), tmax(info.tmax)
	, lightTreshold(info.lightTreshold)
	, sampler(*info.sampler)
{

}

atlas::Renderer::~Renderer()
{

}

atlas::Spectrum atlas::Renderer::getColorAlongRay(const atlas::Ray &r, const atlas::Primitive &scene, int depth)
{
	if (depth <= 0)
		return (atlas::Spectrum(0.f));

	atlas::SurfaceInteraction s;
	if (scene.intersect(r, s))
	{
#if 1//defined(SHADING)
		atlas::sh::BSDF bsdf = s.primitive->getMaterial()->sample(-r.dir, s, sampler.get2D());
		if (bsdf.Li.isBlack())
			return (bsdf.Le);
		return (bsdf.Le + bsdf.pdf * bsdf.Li * getColorAlongRay(atlas::Ray(s.p, bsdf.wi), scene, depth - 1));
#endif
	}
	else
	{
		atlas::Vec3f unitDir = normalize(r.dir);
		Float t = (Float)0.5 * (unitDir.y + (Float)1.0);
		return (((Float)1.0 - t) * atlas::Spectrum(1) + t * atlas::Spectrum((Float)0.5, (Float)0.7, (Float)1.0));
	}
}

void atlas::Renderer::render(const Camera &camera, const Primitive &scene, Film &film)
{
	for (auto pixel : film.croppedPixelBounds)
	{
		atlas::Spectrum colorSum(0);
		sampler.startPixel(pixel);
		for (uint32_t s = 0; s < samplePerPixel; s++)
		{
			atlas::Ray r;
			atlas::CameraSample cs = sampler.getCameraSample(pixel);
			camera.generateRay(cs, r);

			atlas::Spectrum color = getColorAlongRay(r, scene, maxLightBounce);
			colorSum += color;

			film.addSample(cs.pFilm, color, 1);
			sampler.startNextSample();
		}
	}
}