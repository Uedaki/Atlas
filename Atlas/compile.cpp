#include "atlas/Atlas.h"
#include "atlas/core/Logging.h"
#include "atlas/core/Math.h"
#include "atlas/core/Vectors.h"
#include "atlas/core/Points.h"
#include "atlas/core/Ray.h"
#include "atlas/core/Bounds.h"

#include "atlas/core/Primitive.h"

#include "atlas/core/Sampler.h"
#include "atlas/core/Film.h"

#include "atlas/shapes/Sphere.h"

#include "atlas/core/Random.h"
#include "atlas/core/Interaction.h"

#include "atlas/primitives/BvhAccel.h"
#include "atlas/primitives/GeometricPrimitive.h"

#include "atlas/core/Telemetry.h"

#include "NextEventEstimation.h"
#include "atlas/cameras/PerspectiveCamera.h"
#include "atlas/core/ImageIO.h"
#include "nexteventestimation.h"
#include <iostream>
#include <vector>
#include "atlas/core/Light.h"
#include "Acheron.h"
#include "atlas/shapes/Rectangle.h"
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

	std::shared_ptr<atlas::Material> checkerMaterial = std::make_shared<atlas::Material>();
	{
		auto &l = checkerMaterial->addEntryShader<atlas::Lambert>();

		auto &t = checkerMaterial->addShader<atlas::CheckerTexture>();
		l.iR.bind(t.oColor);

		auto &c1 = checkerMaterial->addShader<atlas::ConstantShader<atlas::Spectrum>>();
		c1.value = atlas::Spectrum((Float)0.2, (Float)0.3, (Float)0.1);
		t.iColor1.bind(c1.out);

		auto &c2 = checkerMaterial->addShader<atlas::ConstantShader<atlas::Spectrum>>();
		c2.value = atlas::Spectrum((Float)0.9);
		t.iColor2.bind(c2.out);

		auto &s = checkerMaterial->addShader<atlas::ConstantShader<Float>>();
		s.value = 10;
		t.iScale.bind(s.out);
	}

	std::shared_ptr<atlas::Material> noiseMaterial = std::make_shared<atlas::Material>();
	{
		auto &l = noiseMaterial->addEntryShader<atlas::Lambert>();

		auto &t = noiseMaterial->addShader<atlas::TurbulenceNoiseTexture>();
		l.iR.bind(t.oColor);

		auto &s = noiseMaterial->addShader<atlas::ConstantShader<Float>>();
		s.value = 4;
		t.iScale.bind(s.out);

		auto &d = noiseMaterial->addShader<atlas::ConstantShader<uint32_t>>();
		d.value = 1;
		t.iDepth.bind(d.out);
	}

	std::shared_ptr<atlas::Material> imageMaterial = std::make_shared<atlas::Material>();
	{
		auto &l = imageMaterial->addEntryShader<atlas::Lambert>();

		auto &t = imageMaterial->addShader<atlas::ImageTexture>();
		l.iR.bind(t.oColor);
		t.imageID = atlas::ImageLibrary::requestID("../earthmap.jpg");

		auto &c = imageMaterial->addShader<atlas::ConstantShader<atlas::Vec2f>>();
		c.value = atlas::Vec2f(0, 0);
		t.iOffset.bind(c.out);
	}

	std::shared_ptr<atlas::Material> emissiveMaterial = std::make_shared<atlas::Material>();
	{
		auto &l = emissiveMaterial->addEntryShader<atlas::Emission>();

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
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo),
#if defined(SHADING)
		//checkerMaterial
		noiseMaterial
		//atlas::createLambertMaterial(atlas::Spectrum(0.5))
#else
		atlas::MatteMaterial::create()
#endif
		));
	
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
//				std::shared_ptr<atlas::Material> material = nullptr;
//#else
//				std::shared_ptr<atlas::Material> material = nullptr;
//#endif
//				if (choose_mat < 0.8)
//				{
//#if defined(SHADING)
//					material = atlas::createLambertMaterial(atlas::Spectrum(atlas::random() * atlas::random(), atlas::random() * atlas::random(), atlas::random() * atlas::random()));
//#else
//					atlas::MatteMaterialInfo info;
//					info.kd = atlas::createSpectrumConstant(atlas::random() * atlas::random(), atlas::random() * atlas::random(), atlas::random() * atlas::random());
//					material = atlas::MatteMaterial::create(info);
//#endif
//				}
//				else if (choose_mat < 0.95)
//				{
//#if defined(SHADING)
//					material = atlas::createMetalMaterial(atlas::Spectrum(
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
//					material = atlas::createGlassMaterial(1.5f);
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

	sphereInfo.objectToWorld = setTransform(0, 1, 0);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo),
		//atlas::createGlassMaterial(1.3f)
		atlas::createLambertMaterial(atlas::Spectrum(0.8, 0.2, 0.1))
		));

	sphereInfo.objectToWorld = setTransform(-4, 1, 0);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo),
		atlas::createLambertMaterial(atlas::Spectrum(0.8, 0.2, 0.1))
		//emissiveMaterial// imageMaterial//noiseMaterial //atlas::createLambertMaterial(atlas::Spectrum(0.8, 0.2, 0.1))
		));

	sphereInfo.objectToWorld = setTransform(4, 1, 0);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo),
		//atlas::createMetalMaterial(atlas::Spectrum(0.7, 0.6, 0.5))
		atlas::createLambertMaterial(atlas::Spectrum(0.8, 0.2, 0.1))
		));


	sphereInfo.objectToWorld = setTransform(0, 3, 0);
	*sphereInfo.objectToWorld = *sphereInfo.objectToWorld * atlas::Transform::rotateX(90);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);

	Float intensity = 1;
	atlas::MediumInterface mediumInterface;
	std::shared_ptr<atlas::Shape> shape = std::make_shared<atlas::Rectangle>(*sphereInfo.objectToWorld, *sphereInfo.worldToObject, false);

	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		shape,
		//atlas::createLambertMaterial(atlas::Spectrum(0.8, 0.2, 0.1))
		std::make_shared<atlas::DiffuseAreaLight>(*sphereInfo.objectToWorld, mediumInterface, atlas::Spectrum(1) * intensity, 1, shape)
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
		if (s.primitive->getAreaLight())
			return (dynamic_cast<const atlas::DiffuseAreaLight *>(s.primitive->getAreaLight())->lEmit);

		atlas::BSDFSample bsdf = s.primitive->getMaterial()->sample(-r.dir, s, sampler.get2D());

		//atlas::Spectrum ld = bsdf.scatteringPdf * bsdf.Li * sampleLightSources(s, scene, lights);

		atlas::Ray r(s.p + 0.01f * s.n, bsdf.wi);
		atlas::Spectrum Li = rayColor(r, scene, depth - 1, sampler);
		//std::cout << "li " << bsdf.Li << " & " << Li << " le " << bsdf.Le << " sPdf " << bsdf.scatteringPdf << " pdf " << bsdf.pdf << std::endl;

		return (bsdf.Le + std::abs(bsdf.scatteringPdf) * bsdf.Li * Li / std::abs(bsdf.pdf));
	}
	return (0);
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
#elif 1
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

	std::vector<std::shared_ptr<atlas::Light>> lights;
	for (auto prim : primitives)
	{
		if (prim->getAreaLight())
		{
			lights.emplace_back((atlas::Light *)prim->getAreaLight());
		}
	}

	atlas::NextEventEstimation::Info info;
	info.maxLightBounce = 9;
	info.samplePerPixel = 16;
	info.sampler = sampler;
	atlas::NextEventEstimation nee(info);
	nee.render(camera, bvh, lights, film);


	for (auto &l : lights)
	{
		l.reset();
	}

	{
		film.writeImage();
	}
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
	atlas::Sampler *sampler = atlas::StratifiedSampler::create(samplerInfo);

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