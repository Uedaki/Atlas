#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "../Atlas/includes/Atlas/Scene.h"
#include "../Atlas/includes/Atlas/Camera.h"
#include "../Atlas/includes/Atlas/Buffer.h"
#include "../Atlas/includes/Atlas/rendering/Acceleration.h"

#include "GlobalBins.h"
#include "BatchManager.h"

# ifdef ACHERON_EXPORTS
#   define ACH  __declspec( dllexport )
# else
#   define ACH __declspec( dllimport )
# endif

struct Batch
{
	std::vector<glm::vec3> origins;
	std::vector<glm::vec3> directions;
	std::vector<glm::vec3> colors;
	std::vector<uint32_t> pixelIDs;
	std::vector<uint16_t> sampleIDs;
	std::vector<uint16_t> depths;
	std::vector<float> tNears;

	uint32_t size() const {
		return (origins.size());
	}

	void resize(size_t size)
	{
		origins.resize(size);
		directions.resize(size);
		colors.resize(size);
		pixelIDs.resize(size);
		sampleIDs.resize(size);
		depths.resize(size);
		tNears.resize(size);
	}

	void swap(uint32_t a, uint32_t b)
	{
		std::swap(origins[a], origins[b]);
		std::swap(directions[a], directions[b]);
		std::swap(colors[a], colors[b]);
		std::swap(pixelIDs[a], pixelIDs[b]);
		std::swap(sampleIDs[a], sampleIDs[b]);
		std::swap(depths[a], depths[b]);
		std::swap(tNears[a], tNears[b]);
	}

	NRay operator[](uint32_t idx)
	{
		NRay r;
		r.origin = origins[idx];
		r.dir = directions[idx];
		r.weight = colors[idx];
		r.pixelID = pixelIDs[idx];
		r.sampleID = sampleIDs[idx];
		r.depth = depths[idx];
		r.tNear = tNears[idx];
		return (r);
	}

	void set(const NRay &r, uint32_t idx)
	{
		origins[idx] = r.origin;
		directions[idx] = r.dir;
		colors[idx] = r.weight;
		pixelIDs[idx] = r.pixelID;
		sampleIDs[idx] = r.sampleID;
		depths[idx] = r.depth;
		tNears[idx] = r.tNear;
	}
};

class Acheron
{
public:
	ACH Acheron(const atlas::Scene &scene, uint32_t width, uint32_t height, uint32_t nbrSample);
	ACH ~Acheron();

	ACH void launch(const atlas::Camera &camera, uint32_t nbrThreads);

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
	inline uint32_t getMaxSamples() const {
		return (maxSample);
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

private:
	enum class CommandType : uint8_t
	{
		IDLE = 0,
		GEN_FIRST_RAYS,
		SORT_RAYS,
		COMPUTE_CONE_RAY,
		TRAVERSE,
		SORT_HIT_POINT,
		SHADE_HIT_POINT,
		STOP
	};
	
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t maxSample = 0;

	float tmin = 0.01f;
	float tmax = 100.f;
	int maxDepth = 8;

	std::atomic<CommandType> command = CommandType::IDLE;

	const atlas::Camera *camera;
public:
	atlas::Buffer output;
private:
	uint64_t zOrderMax = 0;
	uint64_t zOrderStep = 516;
	uint32_t currentSampleTarget = 0;

	std::atomic<uint64_t> zOrderPos = 0;

	std::atomic<bool> isRendering = false;
	const atlas::Scene &scene;
	std::vector<atlas::rendering::Acceleration> accelerations;

	std::vector<std::thread> threads;
	std::mutex lock;
	std::condition_variable dispatcher;
	std::atomic<uint8_t> sleepingThread;

	Batch batch;
	BatchManager batchManager;
	GlobalBins bins;

	std::vector<atlas::rendering::HitRecord> hitPoints;

	struct Pack
	{
		std::vector<glm::vec3> *order;
		uint32_t firstIndex;
		uint32_t size;
		uint32_t target;
	};
	std::vector<Pack> sortPack;
	std::atomic<uint32_t> packSize;
	std::atomic<uint32_t> packOffset;
	std::atomic<uint32_t> packIndex;

	void runOneIteration();
	void processRays();

	void runSlave();
	void generateFirstRays();

	uint8_t pickLargestAxis(const std::vector<glm::vec3> &ar, uint32_t firstIndex, uint32_t lastIndex);
	uint32_t partition(std::vector<glm::vec3> &order, uint32_t major, uint32_t firstIndex, uint32_t lastIndex, uint32_t pivotIndex);
	void sortRays();
	void sortNextRay();
	void quicksort();

	void pushCommand(CommandType newCommand);
	void pushCommandForOne(CommandType newCommand);
	void sleepUntilSignal();
};