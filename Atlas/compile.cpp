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

#include "Acheron.h"

#include "Material.h"
#include "Lambert.h"
#include "Metal.h"
#include "Glass.h"

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
		atlas::Sphere::createShape(sphereInfo),
#if defined(SHADING)
		atlas::sh::createLambertMaterial(atlas::Spectrum(0.5))
#else
		atlas::MatteMaterial::create()
#endif
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
#if defined(SHADING)
				std::shared_ptr<atlas::sh::Material> material = nullptr;
#else
				std::shared_ptr<atlas::Material> material = nullptr;
#endif
				if (choose_mat < 0.8)
				{
#if defined(SHADING)
					material = atlas::sh::createLambertMaterial(atlas::Spectrum(atlas::random() * atlas::random(), atlas::random() * atlas::random(), atlas::random() * atlas::random()));
#else
					atlas::MatteMaterialInfo info;
					info.kd = atlas::createSpectrumConstant(atlas::random() * atlas::random(), atlas::random() * atlas::random(), atlas::random() * atlas::random());
					material = atlas::MatteMaterial::create(info);
#endif
				}
				else if (choose_mat < 0.95)
				{
#if defined(SHADING)
					material = atlas::sh::createMetalMaterial(atlas::Spectrum(
						0.5f * (1.0f + atlas::random(),
							0.5f * (1.0f + atlas::random()),
							0.5f * (1.0f + atlas::random()))));
#else
					atlas::MetalMaterialInfo info;
					info.eta = atlas::createSpectrumConstant(
						0.5f * (1.0f + atlas::random(),
						0.5f * (1.0f + atlas::random()), 
						0.5f * (1.0f + atlas::random())));
					material = atlas::MetalMaterial::create(info);
#endif
				}
				else
				{
#if defined(SHADING)
					material = atlas::sh::createGlassMaterial(1.5f);
#else
					material = atlas::GlassMaterial::create();
#endif
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
		atlas::Sphere::createShape(sphereInfo),
#if defined(SHADING)
		atlas::sh::createGlassMaterial(1.3f)
#else
		atlas::GlassMaterial::create(glassInfo)
#endif
		));

	sphereInfo.objectToWorld = setTransform(-4, 1, 0);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo),
#if defined(SHADING)
		atlas::sh::createLambertMaterial(atlas::Spectrum(0.8, 0.2, 0.1))
#else
		material
#endif
		));

	atlas::MetalMaterialInfo metalInfo;
	metalInfo.eta = atlas::createSpectrumConstant(0.7, 0.6, 0.5);
	metalInfo.roughness = atlas::createFloatConstant(0);

	sphereInfo.objectToWorld = setTransform(4, 1, 0);
	sphereInfo.worldToObject = setInverse(sphereInfo.objectToWorld);
	scene.push_back(std::make_shared<atlas::GeometricPrimitive>(
		atlas::Sphere::createShape(sphereInfo),
#if defined(SHADING)
		atlas::sh::createMetalMaterial(atlas::Spectrum(0.7, 0.6, 0.5))
#else
		atlas::MetalMaterial::create(metalInfo)
#endif
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
		if (bsdf.color.isBlack())
			return (bsdf.color);
		return (/* bsdf.pdf * */ bsdf.color * rayColor(atlas::Ray(s.p, bsdf.wi), scene, depth - 1, sampler));
#endif

	}
	atlas::Vec3f unitDir = normalize(r.dir);
	auto t = 0.5 * (unitDir.y + 1.0);
	return ((1.0 - t) * atlas::Spectrum(1.f) + t * atlas::Spectrum(0.5, 0.7, 1.0));
}

bool intersectLineCone(const atlas::Point3f &lp, atlas::Vec3f &ld, const atlas::Point3f &co, const atlas::Vec3f &cd, Float angle);

int main()
{
	//atlas::Point3f lp(-1);
	//atlas::Vec3f ld(1, 1, 5);

	//atlas::Point3f co(0);
	//atlas::Vec3f cd(0, 0, 1);

	//intersectLineCone(lp, ld, co, cd, atlas::radians(30));
	//return (0);

	const uint32_t width = 720;
	const uint32_t height = 500;
	const uint32_t spp = 2;

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

#if 1
	atlas::Acheron::Info achInfo;
	achInfo.resolution = atlas::Point2i(width, height);
	achInfo.spp = spp;
	achInfo.region = screen;
	achInfo.sampler = sampler;
	achInfo.filter = new atlas::BoxFilter(atlas::Vec2f(0.5, 0.5));
	atlas::Acheron ach(achInfo);

	ach.render(camera, bvh);

#else

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
#endif
	printf("Finished\n");
	return (0);
}

using namespace atlas;

// https://www.geometrictools.com/Documentation/IntersectionLineCone.pdf

int findIntersection(Float u0, Float u1, Float v0, Float v1, Float overlap[2])
{
	int numValid;
	if (u1 < v0 || v1 < u0)
	{
		numValid = 0;
	}
	else if (v0 < u1)
	{
		if (u0 < v1)
		{
			overlap[0] = (u0 < v0 ? v0 : u0);
			overlap[1] = (u1 < v1 ? v1 : u1);
			if (overlap[0] < overlap[1])
				numValid = 2;
			else
				numValid = 1;
		}
		else
		{
			overlap[0] = u0;
			overlap[1] = u0;
			numValid = 1;
		}
	}
	else
	{
		overlap[0] = v0;
		overlap[1] = v0;
		numValid = 1;
	}
	return (numValid);
}

bool intersectLineCone(const Point3f &lp, Vec3f &ld, const Point3f &co, const Vec3f &cd, Float angle)
{
	if (dot(ld, cd) < 0)
		ld = -ld;

	Float cosAngle = std::cos(angle);
	Float cosAngleSqr = cosAngle * cosAngle;

	Vector3<Float> PmV = lp - co;
	Float UdU = dot(ld, ld);
	Float DdU = dot(cd, ld);  // >= 0
	Float DdPmV = dot(cd, PmV);
	Float UdPmV = dot(ld, PmV);
	Float PmVdPmV = dot(PmV, PmV);
	Float c2 = DdU * DdU - cosAngleSqr * UdU;
	Float c1 = DdU * DdPmV - cosAngleSqr * UdPmV;
	Float c0 = DdPmV * DdPmV - cosAngleSqr * PmVdPmV;

	if (c2 != 0)
	{
		Float delta = c1 * c1 - c0 * c2;
		if (delta < 0)
		{
			// nothng
		}
		else if (delta > 0)
		{
			Float x = -c1 / c2;
			Float y = (c2 > (Float)0 ? (Float)1 / c2 : (Float)-1 / c2);
			std::array<Float, 2> t = { x - y * sqrt(delta), x + y * sqrt(delta) };

			// Compute the signed heights at the intersection points, h[0] and
			// h[1] with h[0] <= h[1]. The ordering is guaranteed because we
			// have arranged for the input line to satisfy Dot(D,U) >= 0.
			std::array<Float, 2> h = { t[0] * DdU + DdPmV, t[1] * DdU + DdPmV };

			//if (h[0] >= 0)
			{
				if (h[1] > h[0])
				{
					int numValid;
					Float overlap[2];
					numValid = findIntersection(h[0], h[1], 0, 20, overlap);

					if (numValid == 2)
					{
						Float t0 = (overlap[0] - DdPmV) / DdU, t1 = (overlap[1] - DdPmV) / DdU;
						return true;
					}
					else if (numValid == 1)
					{

					}
					else
					{

					}
				}
				else
				{

				}
			}
			//else if (h[1] <= 0)
			//{
			//	// line intersect the negative cone in two points
			//	if (h[1] > h[0])
			//	{
			//		int numValid;
			//		Float overlap[2];
			//		numValid = findIntersection(h[0], h[1], 0, 20, overlap);

			//		if (numValid == 2)
			//		{
			//			Float t0 = (overlap[0] - DdPmV) / DdU, t1 = (overlap[1] - DdPmV) / DdU;
			//			return true;
			//		}
			//		else if (numValid == 1)
			//		{

			//		}
			//		else
			//		{

			//		}
			//	}
			//	else
			//	{

			//	}
			//}
			//else
			//{
			//	// intersect a single point
			//}
		}
		else
		{

		}
	}
	else if (c1 != 0)
	{

	}
	else
	{

	}
}