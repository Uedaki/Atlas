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
#include "atlas/core/Telemetry.h"

#include "atlas/cameras/PerspectiveCamera.h"
#include "atlas/core/ImageIO.h"

#include <iostream>
#include <vector>

#include "Acheron.h"

#include "Material.h"
#include "Lambert.h"
#include "Metal.h"
#include "Glass.h"
#include "Emission.h"
#include "Texture.h"

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

	std::shared_ptr<atlas::sh::Material> checkerMaterial = std::make_shared<atlas::sh::Material>();
	{
		auto &l = checkerMaterial->addShader<atlas::sh::Lambert>();
		checkerMaterial->bind(l);

		auto &t = checkerMaterial->addShader<atlas::sh::CheckerTexture>();
		l.iR.bind(t.oColor);

		auto &c1 = checkerMaterial->addShader<atlas::sh::ConstantShader<atlas::Spectrum>>();
		c1.value = atlas::Spectrum((Float)0.2, (Float)0.3, (Float)0.1);
		t.iColor1.bind(c1.out);

		auto &c2 = checkerMaterial->addShader<atlas::sh::ConstantShader<atlas::Spectrum>>();
		c2.value = atlas::Spectrum((Float)0.9);
		t.iColor2.bind(c2.out);

		auto &s = checkerMaterial->addShader<atlas::sh::ConstantShader<Float>>();
		s.value = 10;
		t.iScale.bind(s.out);
	}

	std::shared_ptr<atlas::sh::Material> noiseMaterial = std::make_shared<atlas::sh::Material>();
	{
		auto &l = noiseMaterial->addShader<atlas::sh::Lambert>();
		noiseMaterial->bind(l);

		auto &t = noiseMaterial->addShader<atlas::sh::TurbulenceNoiseTexture>();
		l.iR.bind(t.oColor);

		auto &s = noiseMaterial->addShader<atlas::sh::ConstantShader<Float>>();
		s.value = 4;
		t.iScale.bind(s.out);

		auto &d = noiseMaterial->addShader<atlas::sh::ConstantShader<uint32_t>>();
		d.value = 1;
		t.iDepth.bind(d.out);
	}

	std::shared_ptr<atlas::sh::Material> imageMaterial = std::make_shared<atlas::sh::Material>();
	{
		auto &l = imageMaterial->addShader<atlas::sh::Lambert>();
		imageMaterial->bind(l);

		auto &t = imageMaterial->addShader<atlas::sh::ImageTexture>();
		l.iR.bind(t.oColor);
		t.imageID = atlas::sh::ImageLibrary::requestID("../earthmap.jpg");

		auto &c = imageMaterial->addShader<atlas::sh::ConstantShader<atlas::Vec2f>>();
		c.value = atlas::Vec2f(0, 0);
		t.iOffset.bind(c.out);
	}

	std::shared_ptr<atlas::sh::Material> emissiveMaterial = std::make_shared<atlas::sh::Material>();
	{
		auto &l = emissiveMaterial->addShader<atlas::sh::Emission>();
		emissiveMaterial->bind(l);

		auto &t = emissiveMaterial->addConstant<atlas::Spectrum>();
		t.value = atlas::Spectrum(1);
		l.iColor.bind(t.out);

		auto &c = emissiveMaterial->addConstant<Float>();
		c.value = 1.f;
		l.iStrength.bind(c.out);
	}

	atlas::SphereInfo sphereInfo;
	sphereInfo.objectToWorld = setTransform(0, -1000, 0);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
	sphereInfo.radius = 1000.f;
	sphereInfo.zMax = 1000.f;
	sphereInfo.zMin = -1000.f;
//	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
//		atlas::Sphere::createShape(sphereInfo),
//#if defined(SHADING)
//		//checkerMaterial
//		noiseMaterial
//		//atlas::sh::createLambertMaterial(atlas::Spectrum(0.5))
//#else
//		atlas::MatteMaterial::create()
//#endif
//		));
//	
//	for (int a = -11; a < 11; a++)
//	{
//		for (int b = -11; b < 11; b++)
//		{
//			Float choose_mat = atlas::random();
//			atlas::Vec3f center(a + 0.9 * atlas::random(), 0.2, b + 0.9 * atlas::random());
//			sphereInfo.objectToWorld = setTransform(center.x, center.y, center.z);
//			sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
//			sphereInfo.radius = 0.2f;
//			if ((center - atlas::Vec3f(4, 0.2, 0)).length() > 0.9)
//			{
//#if defined(SHADING)
//				std::shared_ptr<atlas::sh::Material> material = nullptr;
//#else
//				std::shared_ptr<atlas::Material> material = nullptr;
//#endif
//				if (choose_mat < 0.8)
//				{
//#if defined(SHADING)
//					material = atlas::sh::createLambertMaterial(atlas::Spectrum(atlas::random() * atlas::random(), atlas::random() * atlas::random(), atlas::random() * atlas::random()));
//#else
//					atlas::MatteMaterialInfo info;
//					info.kd = atlas::createSpectrumConstant(atlas::random() * atlas::random(), atlas::random() * atlas::random(), atlas::random() * atlas::random());
//					material = atlas::MatteMaterial::create(info);
//#endif
//				}
//				else if (choose_mat < 0.95)
//				{
//#if defined(SHADING)
//					material = atlas::sh::createMetalMaterial(atlas::Spectrum(
//						0.5f * (1.0f + atlas::random(),
//							0.5f * (1.0f + atlas::random()),
//							0.5f * (1.0f + atlas::random()))));
//#else
//					atlas::MetalMaterialInfo info;
//					info.eta = atlas::createSpectrumConstant(
//						0.5f * (1.0f + atlas::random(),
//						0.5f * (1.0f + atlas::random()), 
//						0.5f * (1.0f + atlas::random())));
//					material = atlas::MetalMaterial::create(info);
//#endif
//				}
//				else
//				{
//#if defined(SHADING)
//					material = atlas::sh::createGlassMaterial(1.5f);
//#else
//					material = atlas::GlassMaterial::create();
//#endif
//				}
//				scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
//					atlas::Sphere::createShape(sphereInfo), material
//					));
//			}
//		}
//	}

	sphereInfo.radius = 1.f;
	sphereInfo.zMax = 1.f;
	sphereInfo.zMin = -1.f;

	std::shared_ptr<atlas::Material> material = nullptr;
	atlas::MatteMaterialInfo matteInfo;
	matteInfo.kd = atlas::createSpectrumConstant((Float)0.8, (Float)0.2, (Float)0.1);
	material = atlas::MatteMaterial::create(matteInfo);

//	atlas::GlassMaterialInfo glassInfo;
//	glassInfo.index = atlas::createFloatConstant(1.3f);
//
//	sphereInfo.objectToWorld = setTransform(0, 1, 0);
//	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
//	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
//		atlas::Sphere::createShape(sphereInfo),
//#if defined(SHADING)
//		atlas::sh::createGlassMaterial(1.3f)
//#else
//		atlas::GlassMaterial::create(glassInfo)
//#endif
//		));

	sphereInfo.objectToWorld = setTransform(0, 0, 0);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo),
#if defined(SHADING)
		atlas::sh::createLambertMaterial(atlas::Spectrum(0.8, 0.2, 0.1))
		//emissiveMaterial// imageMaterial//noiseMaterial //atlas::sh::createLambertMaterial(atlas::Spectrum(0.8, 0.2, 0.1))
#else
		material
#endif
		));

//	atlas::MetalMaterialInfo metalInfo;
//	metalInfo.eta = atlas::createSpectrumConstant(0.7, 0.6, 0.5);
//	metalInfo.roughness = atlas::createFloatConstant(0);
//
//	sphereInfo.objectToWorld = setTransform(4, 1, 0);
//	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
//	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
//		atlas::Sphere::createShape(sphereInfo),
//#if defined(SHADING)
//		atlas::sh::createMetalMaterial(atlas::Spectrum(0.7, 0.6, 0.5))
//#else
//		atlas::MetalMaterial::create(metalInfo)
//#endif
//		));
	return (scene);
}

atlas::Spectrum rayColor(const atlas::Ray &r, const atlas::Primitive &scene, int depth, atlas::Sampler &sampler)
{
	if (depth <= 0)
		return (atlas::Spectrum(0.f));

	atlas::SurfaceInteraction s;
	if (scene.intersect(r, s))
	{
#if !defined(SHADING)
		Float pdf;
		atlas::Vec3f wi;
		s.primitive->computeScatteringFunctions(s, atlas::TransportMode::Radiance, true);
		atlas::Spectrum color = s.bsdf->sampleF(-r.dir, wi, sampler.get2D(), pdf);

		if (color.isBlack())
			return (color);
		return (/* pdf * */ color * rayColor(atlas::Ray(s.p, wi), scene, depth - 1, sampler));
#else
		atlas::sh::BSDF bsdf = s.primitive->getMaterial()->sample(-r.dir, s, sampler.get2D());
		if (bsdf.Li.isBlack())
			return (bsdf.Le);
		return (bsdf.Le + /* bsdf.pdf * */ bsdf.Li * rayColor(atlas::Ray(s.p, bsdf.wi), scene, depth - 1, sampler));
#endif

	}
	atlas::Vec3f unitDir = normalize(r.dir);
	Float t = (Float)0.5 * (unitDir.y + (Float)1.0);
	return (((Float)1.0 - t) * atlas::Spectrum(1) + t * atlas::Spectrum((Float)0.5, (Float)0.7, (Float)1.0));
}

int main()
{
	const uint32_t width = 720;
	const uint32_t height = 500;
	const uint32_t spp = 16;

#if 0
	// setup rendering configuration
	atlas::StratifiedSampler sampler;

	atlas::Acheron::Info achInfo;
	achInfo.samplePerPixel = spp;
	achInfo.minLightBounce = 0;
	achInfo.maxLightBounce = 16;
	achInfo.lightTreshold = (Float)0.01;
	achInfo.sampler = &sampler;
	achInfo.assetFolder = "./";
	achInfo.temporaryFolder = "./render/";
	atlas::Acheron ach(achInfo);

	// the renderer is setup and running
	// now we can create a camera and a scene for rendering
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

	std::vector<std::shared_ptr<atlas::Primitive>> primitives;// = createPrimitives();
	atlas::BvhAccel bvh(primitives);

	ach.render(camera, bvh, film);

	film.writeImage();
#else
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
	atlas::Sampler *sampler = atlas::TestSampler::create();// atlas::StratifiedSampler::create(samplerInfo);

	{
		TELEMETRY(brute, "brute force");
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

				film.addSample(cs.pFilm, color, 1);
				sampler->startNextSample();
			}

			rgb[(pixel.x + pixel.y * width) * 3] = sqrt(colorSum.r * invSpp);
			rgb[(pixel.x + pixel.y * width) * 3 + 1] = sqrt(colorSum.g * invSpp);
			rgb[(pixel.x + pixel.y * width) * 3 + 2] = sqrt(colorSum.b * invSpp);
		}

		film.writeImage();
	}
#endif
	printf("Finished\n");
	PRINT_TELEMETRY_REPORT();
	return (0);
}

// https://www.geometrictools.com/Documentation/IntersectionLineCone.pdf