#include "pch.h"
#include "Acheron.h"

#include <iostream>

#include "Material.h"
#include "Rendering/HitRecord.h"
#include "Rendering/Ray.h"
#include "Tools.h"

Acheron::Acheron(const Scene &s, uint32_t w, uint32_t h, uint32_t nbrS)
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
	accelerations.reserve(scene.getShapes().size() * 2);
	accelerations.emplace_back();
	accelerations.back().feed(elements, bound, accelerations);

	output.resize(w * h);

	report.accBuildTime.stop();
}

Acheron::~Acheron()
{
	isRendering = false;
	for (auto &thread : threads)
	{
		if (thread.joinable())
			thread.join();
	}
}

void Acheron::launch(const Camera &camera, Buffer &out)
{
	isRendering = true;

	Chronometer chrono;
	chrono.start();

	for (size_t i = 0; i < NBR_THREAD; i++)
	{
		threads.emplace_back([this]() { this->threadJob(); });
	}

	samples.resize(blockSize * blockSize * nbrSample);
	tiles.resize((blockSize * blockSize * nbrSample) / tileCount);

	for (uint32_t i = 0; i < tiles.size(); i++)
	{
		tiles[i].firstIndex = i * tileCount;
		tiles[i].size = (i + 1) * tileCount > (blockSize * blockSize * nbrSample) ? (blockSize * blockSize * nbrSample) - tiles[i].firstIndex : tileCount;
	}

	for (size_t y0 = 0; y0 < height; y0 += blockSize)
	{
		for (size_t x0 = 0; x0 < width; x0 += blockSize)
		{
			batchSampleCount = 0;

			for (size_t y = 0; y < blockSize && y0 + y < height; y++)
			{
				for (size_t x = 0; x < blockSize && x0 + x < width; x++)
				{
					for (size_t s = 0; s < nbrSample; s++)
					{
						//uint32_t index = s + x * nbrSample + y * (nbrSample * blockSize);
						Sample sample;
						sample.x = x0 + x;
						sample.y = y0 + y;

						const float u = (static_cast<float>(x0 + x) + Tools::rand()) / width;
						const float v = (static_cast<float>(y0 + y) + Tools::rand()) / height;

						sample.ray = camera.getRay(u, v);
						samples[batchSampleCount] = sample;
						batchSampleCount += 1;
					}
				}
			}

			activeTile = tiles.size();
			for (uint32_t i = 0; i < tiles.size(); i++)
			{
				if (tiles[i].firstIndex >= batchSampleCount)
				{
					activeTile = i;
					break;
				}
			}

			activeSample = batchSampleCount;
			processTiles();
			
			fetchResult(out);
			//chrono.stop();
			//std::cout << "Rendering finished in " << chrono << std::endl;
			//mode = 1;
			//return;
		}
	}

	chrono.stop();
	std::cout << "Rendering finished in " << chrono << std::endl;
	mode = 1;
}

void Acheron::processTiles()
{
	depth = 0;
	//activeTile = tiles.size();
	//activeSample = samples.size();
	while (activeTile > 0 && depth < maxDepth)
	{
		nextTile.store(0);
		finishedThread.store(0);
		mode.store(2);

		traverse();

		while (finishedThread != NBR_THREAD + 1)
		{
		}

		// process finished tiled

		uint32_t firstLostSample = activeSample;
		for (uint32_t i = 0; i < firstLostSample; i++)
		{
			if (samples[i].hit.t < 0)
			{
				firstLostSample -= 1;
				Sample tmp = samples[i];
				samples[i] = samples[firstLostSample];
				samples[firstLostSample] = tmp;
				i = i - 1;
			}
		}

		//std::cout << "Shade " << std::endl;

		nextTile.store(0);
		finishedThread.store(0);
		mode.store(3);

		shade();

		while (finishedThread != NBR_THREAD + 1)
		{
		}

		activeSample = firstLostSample;
		for (uint32_t i = 0; i < activeTile; i++)
		{
			if (tiles[i].firstIndex >= firstLostSample)
			{
				activeTile = i;
				break;
			}
		}

		std::cout << "depth " << depth << " computed, " << activeTile << " tiles remaining" << std::endl;
		depth += 1;
		//return;
	}
}


void Acheron::threadJob()
{
	while (mode != 1)
	{
		if (mode == 2 && nextTile < activeTile)
		{
			traverse();
		}
		else if (mode == 3 && nextTile < activeTile)
		{
			shade();
		}
	}
}

void Acheron::traverse()
{
	//std::cout << "start traverse with " << nextTile << " and " << activeTile << std::endl;
	while (nextTile < activeTile)
	{
		uint32_t tileIndex = nextTile.fetch_add(1);

		Tile &tile = tiles[tileIndex];
		for (uint32_t sampleIndex = tile.firstIndex; sampleIndex < tile.firstIndex + tile.size; sampleIndex++)
		{
			Sample &sample = samples[sampleIndex];
			sample.hit = HitRecord();
			accelerations.front().hit(sample.ray, tmin, tmax, sample.hit);
		}
	}
	finishedThread.fetch_add(1);
	mode.store(0);
}

void Acheron::shade()
{
	//std::cout << "start shade with " << nextTile << " and " << activeTile << std::endl;
	while (nextTile < activeTile)
	{
		uint32_t tileIndex = nextTile.fetch_add(1);
		//std::cout << "Handling tile " << tileIndex << std::endl;
		Tile &tile = tiles[tileIndex];
		for (uint32_t sampleIndex = tile.firstIndex; sampleIndex < tile.firstIndex + tile.size; sampleIndex++)
		{
			Sample &sample = samples[sampleIndex];
			if (sample.hit.material)
			{
				Ray scattered;
				glm::vec3 attenuation;
				glm::vec3 emitted = sample.hit.material->emitted(sample.hit);
				if (sample.hit.material->scatter(sample.ray, sample.hit, attenuation, scattered))
				{
					sample.ray = scattered;
					sample.color *= attenuation;
					//std::cout << &sample << " x " << sample.x << " y " << sample.y << " color " << sample.color.x << ", "<< sample.color.y<< ", "<< sample.color.z << std::endl;
				}
				else
				{
					sample.color *= emitted;
					//std::cout << &sample << " x " << sample.x << " y " << sample.y << " lost" << std::endl;
				}
			}
			else
			{
				glm::vec3 direction = glm::normalize(sample.ray.dir);
				float t = 0.5f * (direction.y + 1);
				sample.color *= (1 - t) * glm::vec3(1, 1, 1) + t * glm::vec3(0.5, 0.7, 1);
				//std::cout << &sample << " x " << sample.x << " y " << sample.y << " sky" << std::endl;
			}
		}
	}
	finishedThread.fetch_add(1);
	mode.store(0);
	//std::cout << "exiting shade with " << nextTile << " and " << activeTile << std::endl;
}

void Acheron::fetchResult(Buffer &dst)
{
	//if (!isRendering)
	//	return;

	std::cout << "------------- Fetch ----------------" << std::endl;
		
	float invSampleCount = 1.f / nbrSample;


	for (uint32_t i = 0; i < batchSampleCount; i++)
	{
		//std::cout << &samples[i] << " x " << samples[i].x << " y " << samples[i].y << " color " << samples[i].color.x << ", " << samples[i].color.y << ", " << samples[i].color.z << std::endl;
		uint32_t index = samples[i].x + samples[i].y * width;
		glm::vec3 color = samples[i].color * invSampleCount;
		dst.image[index] += color;
	}

	//isRendering = false;
}