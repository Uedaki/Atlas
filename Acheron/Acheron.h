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
		TRAVERSE,
		SHADE,
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

	Batch batch;
	BatchManager batchManager;
	GlobalBins bins;

	void runOneIteration();
	void processRays();

	void runSlave();
	void generateFirstRays();

	void pushCommand(CommandType newCommand);
	void sleepUntilSignal();
};