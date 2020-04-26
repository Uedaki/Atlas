#include "pch.h"
#include "Rendering/Session.h"

#include <iostream>

#include "Material.h"
#include "Rendering/HitRecord.h"
#include "Rendering/Ray.h"
#include "Tools.h"

atlas::rendering::Session::Session(const Scene &s, uint32_t w, uint32_t h, uint32_t nbrS)
	: scene(s)
	, width(w), height(h), nbrSample(nbrS)
{
	report.accBuildTime.start();

	bool isFirst = true;
	Bound bound;
	std::vector<const Hitable *> elements;
	for (auto &shape : scene.getShapes())
	{
		elements.emplace_back(shape.get());
		if (isFirst)
		{
			bound = shape->getBound();
			isFirst = false;
		}
		else
			bound += shape->getBound();
	}
	accelerations.reserve(scene.getShapes().size());
	accelerations.emplace_back();
	accelerations.back().feed(elements, bound, accelerations);

	report.accBuildTime.stop();
}

atlas::rendering::Session::~Session()
{
	isRendering = false;
	for (auto &thread : threads)
	{
		if (thread.joinable())
			thread.join();
	}
}

void atlas::rendering::Session::launch(const Camera &camera, int nbrThread)
{
	assert(nbrThread > 0);

	report.renderingTime.start();

	isRendering = true;
	nbrThreadWorking = nbrThread;
	slaves.resize(nbrThread);
	generateRenderingJobs();
	for (size_t i = 0; i < nbrThread; i++)
	{
		slaves[i].buffer[0].resize(blockSize * blockSize);
		slaves[i].buffer[1].resize(blockSize * blockSize);
		threads.emplace_back([this](const Camera *camera, Slave *slave) { this->processJobs(*camera, *slave); },
			&camera, &slaves[i]);
	}
}

void atlas::rendering::Session::launch(const Camera &camera, Buffer &out)
{
	Region region;
	region.left = 0;
	region.right = width;
	region.bottom = 0;
	region.top = height;
	region.currNbrSample = 0;
	region.targetNbrSample = nbrSample;
	renderRegion(camera, region, out);
}

void atlas::rendering::Session::generateRenderingJobs()
{
	size_t step = 1;
	for (size_t s = 0; s < nbrSample; s += step / 2)
	{
		for (size_t y = 0; y < height; y += blockSize)
		{
			for (size_t x = 0; x < width; x += blockSize)
			{
				jobs.emplace();
				Region &region = jobs.back();
				region.left = x;
				region.right = x + blockSize <= width ? x + blockSize : width;
				region.bottom = y;
				region.top = y + blockSize <= height ? y + blockSize : height;
				region.currNbrSample = s;
				region.targetNbrSample = s + step < nbrSample ? s + step : nbrSample;
			}
		}
		step *= 2;
	}
}

void atlas::rendering::Session::processJobs(const Camera &camera, Slave &slave)
{
	while (true)
	{
		mutex.lock();
		if (jobs.empty())
		{
			mutex.unlock();
			nbrThreadWorking--;
			return;
		}
		slave.region[slave.renderingBufferIdx] = jobs.front();
		jobs.pop();
		printf("jobs left %zd\n", jobs.size());
		mutex.unlock();

		renderRegion(camera, slave.region[slave.renderingBufferIdx], slave.buffer[slave.renderingBufferIdx]);

		if (!isRendering)
			return;

		slave.mutex.lock();
		if (!slave.hasBeenCollected)
		{
			slave.mutex.unlock();
			while (true)
			{
				;// std::this_thread::sleep_for(); // sleep
				slave.mutex.lock();
				if (slave.hasBeenCollected)
					break;
				slave.mutex.unlock();
			}
		}

		slave.hasBeenCollected = false;
		slave.renderingBufferIdx = (slave.renderingBufferIdx + 1) % 2;
		slave.mutex.unlock();
	}
}

void atlas::rendering::Session::fetchResult(Buffer &dst)
{
	if (!isRendering)
		return;

	for (auto &slave : slaves)
	{
		slave.mutex.lock();
		if (!slave.hasBeenCollected)
		{
			uint8_t idx = (slave.renderingBufferIdx + 1) % 2;
			for (size_t y = 0; y < slave.region[idx].top - slave.region[idx].bottom; y++)
			{
				for (size_t x = 0; x < slave.region[idx].right - slave.region[idx].left; x++)
				{
					const float lastNbrSample = static_cast<float>(slave.region[idx].currNbrSample);
					const float newNbrSample = static_cast<float>(slave.region[idx].targetNbrSample - slave.region[idx].currNbrSample);

					const size_t slaveIdx = x + y * (slave.region[idx].right - slave.region[idx].left);
					const size_t dstIdx = slave.region[idx].left + x + (slave.region[idx].bottom + y) * width;
					const float invMaxSample = 1.f / slave.region[idx].targetNbrSample;

					const glm::vec3 oldColor = dst.image[dstIdx];
					const glm::vec3 oldAlbedo = dst.albedo[dstIdx];
					const glm::vec3 oldNormal = dst.normal[dstIdx];

					const glm::vec3 color = slave.buffer[idx].image[slaveIdx];
					const glm::vec3 albedo = slave.buffer[idx].albedo[slaveIdx];
					const glm::vec3 normal = slave.buffer[idx].normal[slaveIdx];

					dst.image[dstIdx] = (oldColor * lastNbrSample + color * newNbrSample) * invMaxSample;
					dst.albedo[dstIdx] = (oldAlbedo * lastNbrSample + albedo * newNbrSample) * invMaxSample;
					dst.normal[dstIdx] = (oldNormal * lastNbrSample + normal * newNbrSample) * invMaxSample;

				}
			}
			slave.hasBeenCollected = true;
		}
		slave.mutex.unlock();
	}

	if (nbrThreadWorking == 0)
	{
		isRendering = false;
		report.renderingTime.stop();
		std::cout << "Rendering finished in " << report.renderingTime << std::endl;

		for (auto &thread : threads)
		{
			thread.join();
		}
	}
}

void atlas::rendering::Session::renderRegion(const Camera &camera, const Region &region, Buffer &dst)
{
	const float invSample = 1.f / (region.targetNbrSample - region.currNbrSample);

#ifndef TRUE
	for (size_t y = region.bottom; y < region.top; y++)
	{
		for (size_t x = region.left; x < region.right; x++)
		{
			Texel texel;
			for (size_t s = 0; s < (region.targetNbrSample - region.currNbrSample); s++)
			{
				const float u = (static_cast<float>(x) + Tools::rand()) / width;
				const float v = (static_cast<float>(y) + Tools::rand()) / height;

				const Ray ray = camera.getRay(u, v);
				texel += traceRay(ray);
			}

			size_t index = x - region.left + (y - region.bottom) * (region.right - region.left);
			dst.image[index] = texel.color * invSample;
			dst.albedo[index] = texel.albedo * invSample;
			dst.normal[index] = texel.normal * invSample;
		}
	}
#else
	for (size_t y = region.bottom; y < region.top; y += 2)
	{
		for (size_t x = region.left; x < region.right; x += 2)
		{
			SimdTexel texel;
			texel.r = float4(0.f);
			texel.g = float4(0.f);
			texel.b = float4(0.f);
			for (size_t s = 0; s < (region.targetNbrSample - region.currNbrSample); s++)
			{
				float4 u((static_cast<float>(x) + Tools::rand()) / width,
					(static_cast<float>(x + 1) + Tools::rand()) / width,
					(static_cast<float>(x) + Tools::rand()) / width,
					(static_cast<float>(x + 1) + Tools::rand()) / width);
				float4 v((static_cast<float>(y) + Tools::rand()) / height,
					(static_cast<float>(y) + Tools::rand()) / height,
					(static_cast<float>(y + 1) + Tools::rand()) / height,
					(static_cast<float>(y + 1) + Tools::rand()) / height);

				SimdRay ray = camera.getSimdRay(u, v);
	
				SimdTexel newTexel = traceSimdRay(ray);
				texel.r = texel.r + newTexel.r;
				texel.g = texel.g + newTexel.g;
				texel.b = texel.b + newTexel.b;
			}

			float r[4];
			_mm_store_ps(r, texel.r.m);

			float g[4];
			_mm_store_ps(g, texel.g.m);

			float b[4];
			_mm_store_ps(b, texel.b.m);


			size_t index = x - region.left + (y - region.bottom) * (region.right - region.left);
			size_t index2 = x - region.left + (y + 1 - region.bottom) * (region.right - region.left);
			dst.image[index] = glm::vec3(r[0], g[0], b[0]) * invSample;
			dst.image[index + 1] = glm::vec3(r[1], g[1], b[1]) * invSample;
			dst.image[index2] = glm::vec3(r[2], g[2], b[2]) * invSample;
			dst.image[index2 + 1] = glm::vec3(r[3], g[3], b[3]) * invSample;
		}
	}
#endif
}

atlas::rendering::Texel atlas::rendering::Session::traceRay(const Ray &ray, int depth) const
{
	Texel texel;
	HitRecord hit;
	if (accelerations.front().hit(ray, tmin, tmax, hit))
	{
		texel.color = glm::vec3(1, 1, 1);
		return texel;
		Ray scattered;
		glm::vec3 attenuation;
		texel.normal = hit.normal;
		glm::vec3 emitted = hit.material->emitted(hit);
		texel.albedo = emitted + attenuation;
		if (depth < maxDepth && hit.material->scatter(ray, hit, attenuation, scattered))
		{
			texel.color = emitted + attenuation * traceRay(scattered, depth + 1).color;
		}
		else
		{
			texel.color = emitted;
		}
	}
	else
	{
		texel.color = glm::vec3(0.5, 0.5, 0.5);
		return (texel);

		glm::vec3 direction = glm::normalize(ray.dir);
		float t = 0.5f * (direction.y + 1);
		texel.albedo = (1 - t) * glm::vec3(1, 1, 1) + t * glm::vec3(0.5, 0.7, 1);
		texel.color = texel.albedo;
	}
	return (texel);
}

SimdTexel atlas::rendering::Session::traceSimdRay(SimdRay &ray, int depth) const
{
	SimdTexel texel;
	SimdHitRecord hit;

	float4 min(tmin);
	float4 max(tmax);

	bool4 msk = accelerations.front().simdHit(ray, min, max, hit);

	texel.r = select(float4(0.5), hit.texel.r, msk);
	texel.g = select(float4(0.5), hit.texel.g, msk);
	texel.b = select(float4(0.5), hit.texel.b, msk);
	return (texel);
}