#include "atlas/core/Light.h"

#include "atlas/core/primitive.h"
#include "atlas/core/RgbSpectrum.h"
#include "atlas/core/Sampling.h"

bool atlas::VisibilityTester::unoccluded(const Primitive &scene) const
{
	return (!scene.intersectP(p0.spawnRayTo(p1)));
}

atlas::Spectrum atlas::VisibilityTester::tr(const Primitive &scene, Sampler &sampler) const
{
	Ray ray(p0.spawnRayTo(p1));
	Spectrum tr(1.f);
	while (true)
	{
		SurfaceInteraction isect;
		bool hitSurface = scene.intersect(ray, isect);
		if (hitSurface && isect.primitive->getMaterial() != nullptr)
			return (Spectrum(0));

		if (ray.medium)
			tr *= ray.medium->tr(ray, sampler);

		if (!hitSurface)
			break;
		ray = isect.spawnRayTo(p1);
	}
	return (tr);
}

atlas::Spectrum atlas::DiffuseAreaLight::l(const Interaction &intr, const Vec3f &w) const
{
	return (dot(intr.n, w) > 0.f ? lEmit : Spectrum(0));
}

atlas::Spectrum atlas::DiffuseAreaLight::power() const
{
	return (lEmit * area * PI);
}

atlas::Spectrum atlas::DiffuseAreaLight::sampleLi(const Interaction &ref, const Point2f &u, Vec3f &wi, Float &pdf, VisibilityTester &vis) const
{
	Interaction pShape = shape->sample(ref, u, pdf);
	pShape.mediumInterface = mediumInterface;
	if (pdf == 0 || (pShape.p - ref.p).lengthSquared() == 0)
	{
		pdf = 0;
		return 0.f;
	}
	wi = normalize(pShape.p - ref.p);
	vis = VisibilityTester(ref, pShape);
	return l(pShape, -wi);
}

Float atlas::DiffuseAreaLight::pdfLi(const Interaction &ref, const Vec3f &wi) const
{
	return shape->pdf(ref, wi);
}

atlas::Spectrum atlas::DiffuseAreaLight::sampleLe(const Point2f &u1, const Point2f &u2, Float time, Ray &ray, Normal &nLight, Float &pdfPos, Float &pdfDir) const
{
	// Sample a point on the area light's _Shape_, _pShape_
	Interaction pShape = shape->sample(u1, pdfPos);
	pShape.mediumInterface = mediumInterface;
	nLight = pShape.n;

	// Sample a cosine-weighted outgoing direction _w_ for area light
	Vec3f w;
	w = cosineSampleHemisphere(u2);
	pdfDir = cosineHemispherePdf(w.z);

	Vec3f v1, v2, n(pShape.n);
	coordinateSystem(n, &v1, &v2);
	w = w.x * v1 + w.y * v2 + w.z * n;
	ray = pShape.spawnRay(w);
	return l(pShape, w);
}

void atlas::DiffuseAreaLight::pdfLe(const Ray &ray, const Normal &nLight, Float &pdfPos, Float pdfDir) const
{
	Interaction it(ray.origin, nLight, Vec3f(), Vec3f(nLight), ray.time,
		mediumInterface);
	pdfPos = shape->pdf(it);
	pdfDir = cosineHemispherePdf(dot(nLight, ray.dir));
}