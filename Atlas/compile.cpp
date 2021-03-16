#include "atlas/Atlas.h"
#include "atlas/core/Logging.h"
#include "atlas/core/Math.h"
#include "atlas/core/Vectors.h"
#include "atlas/core/Points.h"
#include "atlas/core/Ray.h"
#include "atlas/core/Bounds.h"

#include "atlas/core/Material.h"
#include "atlas/core/Primitive.h"

#include "atlas/core/Sampler.h"
#include "atlas/core/Film.h"

#include "atlas/shapes/Sphere.h"

#include "atlas/materials/Glass.h"
#include "atlas/materials/Metal.h"

#include "atlas/core/Random.h"
#include "atlas/core/Interaction.h"

#include "atlas/primitives/BvhAccel.h"
#include "atlas/primitives/GeometricPrimitive.h"

#include "atlas/core/Texture.h"

#include "atlas/cameras/PerspectiveCamera.h"
#include "atlas/core/ImageIO.h"

#include <iostream>
#include <vector>

atlas::Transform *setTransform(Float x, Float y, Float z)
{
	atlas::Transform *tr = new atlas::Transform;
	(*tr) = atlas::Transform::translate(atlas::Vec3f(x, y, z));
	return (tr);
}

atlas::Transform *setInverse(const atlas::Transform *tr)
{
	atlas::Transform *inv = new atlas::Transform;
	(*inv) = tr->inverse();
	return (inv);
}

std::vector<std::shared_ptr<atlas::Primitive>> createPrimitives()
{
	std::vector<std::shared_ptr<atlas::Primitive>> scene;

	atlas::SphereInfo sphereInfo;
	sphereInfo.objectToWorld = setTransform(0, -1000, 0);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
	sphereInfo.radius = 1000.f;
	sphereInfo.zMax = 1000.f;
	sphereInfo.zMin = -1000.f;
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo), atlas::MatteMaterial::create()
		));
	
	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			Float choose_mat = atlas::random();
			atlas::Vec3f center(a + 0.9 * atlas::random(), 0.2, b + 0.9 * atlas::random());
			sphereInfo.objectToWorld = setTransform(center.x, center.y, center.z);
			sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
			sphereInfo.radius = 0.2f;
			if ((center - atlas::Vec3f(4, 0.2, 0)).length() > 0.9)
			{
				std::shared_ptr<atlas::Material> material = nullptr;
				if (choose_mat < 0.8)
				{
					atlas::MatteMaterialInfo info;
					info.kd = atlas::createSpectrumConstant(atlas::random() * atlas::random(), atlas::random() * atlas::random(), atlas::random() * atlas::random());
					material = atlas::MatteMaterial::create(info);
				}
				else if (choose_mat < 0.95)
				{
					atlas::MetalMaterialInfo info;
					info.eta = atlas::createSpectrumConstant(
						0.5f * (1.0f + atlas::random(),
						0.5f * (1.0f + atlas::random()), 
						0.5f * (1.0f + atlas::random())));
					material = atlas::MetalMaterial::create(info);
				}
				else
				{
					material = atlas::GlassMaterial::create();
				}
				scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
					atlas::Sphere::createShape(sphereInfo), material
					));
			}
		}
	}

	sphereInfo.radius = 1.f;
	sphereInfo.zMax = 1.f;
	sphereInfo.zMin = -1.f;

	std::shared_ptr<atlas::Material> material = nullptr;
	atlas::MatteMaterialInfo matteInfo;
	matteInfo.kd = atlas::createSpectrumConstant(0.8, 0.2, 0.1);
	material = atlas::MatteMaterial::create(matteInfo);

	atlas::GlassMaterialInfo glassInfo;
	glassInfo.index = atlas::createFloatConstant(1.3f);

	sphereInfo.objectToWorld = setTransform(0, 1, 0);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo), atlas::GlassMaterial::create(glassInfo)
		));

	sphereInfo.worldToObject = setTransform(-4, 1, 0);
	sphereInfo.objectToWorld = setInverse(sphereInfo.objectToWorld);
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo), material
		));

	atlas::MetalMaterialInfo metalInfo;
	metalInfo.eta = atlas::createSpectrumConstant(0.7, 0.6, 0.5);
	metalInfo.roughness = atlas::createFloatConstant(0);

	sphereInfo.objectToWorld = setTransform(4, 1, 0);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo), atlas::MetalMaterial::create(metalInfo)
		));

	return (scene);
}

atlas::Spectrum rayColor(const atlas::Ray &r, const atlas::Primitive &scene, int depth, atlas::Sampler &sampler)
{
	if (depth <= 0)
		return (atlas::Spectrum(0.f));

	atlas::SurfaceInteraction s;
	if (scene.intersect(r, s))
	{
		Float pdf;
		atlas::Vec3f wi;
		s.primitive->computeScatteringFunctions(s, atlas::TransportMode::Radiance, true);
		atlas::Spectrum color = s.bsdf->sampleF(-r.dir, wi, sampler.get2D(), pdf);
		if (color.isBlack())
			return (color);
		return (/* pdf * */ color * rayColor(atlas::Ray(s.p, wi), scene, depth - 1, sampler));
	}
	atlas::Vec3f unitDir = normalize(r.dir);
	auto t = 0.5 * (unitDir.y + 1.0);
	return ((1.0 - t) * atlas::Spectrum(1.f) + t * atlas::Spectrum(0.5, 0.7, 1.0));
}

int main()
{
	const uint32_t width = 720;
	const uint32_t height = 500;
	const uint32_t spp = 8;

	atlas::FilmInfo filmInfo;
	filmInfo.filename = "film.ppm";
	filmInfo.resolution = atlas::Point2i(width, height);
	filmInfo.filter = new atlas::BoxFilter(atlas::Vec2f(0.5, 0.5));
	atlas::Film film(filmInfo);

	atlas::Bounds2f screen;
	screen.min.x = -1.f;
	screen.max.x = 1.f;
	screen.min.y = -1.f;
	screen.max.y = 1.f;
	atlas::Transform worldToCam = atlas::Transform::lookAt(atlas::Point3f(13, 2, 3), atlas::Point3f(0, 0, 0), atlas::Vec3f(0, 1, 0));
	atlas::PerspectiveCamera camera(worldToCam.inverse(), screen, 0.f, 1.f, 0, 10, 40, &film, nullptr);

	std::vector<std::shared_ptr<atlas::Primitive>> primitives = createPrimitives();
	atlas::BvhAccel bvh(primitives);

	atlas::StratifiedSamplerInfo samplerInfo;
	atlas::Sampler *sampler = atlas::StratifiedSampler::create(samplerInfo);

	Float invSpp = 1.f / spp;
	Float *rgb = new Float[width * height * 3];
	for (auto pixel : film.croppedPixelBounds)
	{
		atlas::Spectrum colorSum(0);
		sampler->startPixel(pixel);
		for (uint32_t s = 0; s < spp; s++)
		{
			atlas::Ray r;
			atlas::CameraSample cs = sampler->getCameraSample(pixel);
			camera.generateRay(cs, r);

			atlas::Spectrum color = rayColor(r, bvh, 9, *sampler);
			colorSum += color;

			film.addSample(pixel, cs.pFilm, color, 1);
			sampler->startNextSample();
		}

		rgb[(pixel.x + pixel.y * width) * 3] = sqrt(colorSum.r * invSpp);
		rgb[(pixel.x + pixel.y * width) * 3 + 1] = sqrt(colorSum.g * invSpp);
		rgb[(pixel.x + pixel.y * width) * 3 + 2] = sqrt(colorSum.b * invSpp);
	}

	film.writeImage();
}