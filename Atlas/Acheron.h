#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "Atlas/Rendering/Acceleration.h"
#include "Atlas/Rendering/Ray.h"
#include "Atlas/Camera.h"
#include "Atlas/Buffer.h"
#include "Atlas/Chronometer.h"
#include "Atlas/Scene.h"
#include "Atlas/Rendering/Texel.h"

#include "Atlas/rendering/HitRecord.h"

#include "atlas/rendering/Ray.h"

using namespace atlas;
using namespace atlas::rendering;

struct Sample
{
	float x;
	float y;

	atlas::rendering::Ray ray;
	atlas::rendering::HitRecord hit;

	glm::vec3 color = glm::vec3(1, 1, 1);
};

struct Tile
{
	uint32_t firstIndex;
	uint32_t size;
};

class Acheron
{
public:
	struct Region
	{
		size_t left;
		size_t right;
		size_t bottom;
		size_t top;
		size_t currNbrSample;
		size_t targetNbrSample;
	};

	struct Report
	{
		Chronometer accBuildTime;
		Chronometer renderingTime;
	};

	ATLAS Acheron(const Scene &scene, uint32_t width, uint32_t height, uint32_t nbrSample);
	ATLAS ~Acheron();

	ATLAS void launch(const Camera &camera, Buffer &out);

	ATLAS void fetchResult(Buffer &dst);

	inline operator bool() const {
		return (isRendering);
	}

	template <typename T = uint32_t>
	inline T getWidth() const {
		return (static_cast<T>(width));
	}

	template <typename T = uint32_t>
	inline T getHeight() const {
		return (static_cast<T>(height));
	}

	inline float getOutputImageRatio() const {
		return (static_cast<float>(width) / height);
	}
	inline uint32_t getSamples() const {
		return (nbrSample);
	}

	inline void setNear(float n) {
		tmin = n;
	}
	inline void setFar(float f) {
		tmax = f;
	}
	inline void setMaxDepth(int d) {
		maxDepth = d;
	}
	inline void setBlockSize(uint32_t size) {
		blockSize = size;
	}
	inline bool isWorking() const {
		return (isRendering);
	}
	inline float getNear() const {
		return (tmin);
	};
	inline float getFar() const {
		return (tmax);
	};
	inline int getMaxDepth() const {
		return (maxDepth);
	};
	inline uint32_t getBlockSize() const {
		return (blockSize);
	};

private:
	std::atomic<bool> isRendering = false;

	Chronometer chrono;

	const Scene &scene;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t nbrSample = 0;

	std::vector<Acceleration> accelerations;

	std::atomic<uint8_t> nbrThreadWorking;

	float tmin = 0.01f;
	float tmax = 100.f;
	int maxDepth = 8;
	uint32_t blockSize = 256;

	Report report;

	public:

		std::mutex locker;

	uint32_t depth;
	std::atomic<uint8_t> mode;
	std::atomic<uint32_t> nextTile;
	std::atomic<uint32_t> finishedThread;

	uint32_t batchSampleCount;
	uint32_t activeTile;
	uint32_t activeSample;
	uint32_t tileCount = 64;

#define NBR_THREAD 3

	std::vector<std::thread> threads;

	std::vector<Sample> samples;
	std::vector<Tile> tiles;
	Buffer output;

	void processTiles();
	void threadJob();

	void traverse();
	void shade();
};