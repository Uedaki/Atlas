#pragma once

# ifdef ATLAS_EXPORT
#   define ATLAS  __declspec( dllexport )
# else
#   define ATLAS __declspec( dllimport )
# endif

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "Atlas/Rendering/Acceleration.h"
#include "Atlas/Camera.h"
#include "Atlas/Buffer.h"
#include "Atlas/Chronometer.h"
#include "Atlas/Scene.h"
#include "Atlas/Rendering/Texel.h"

namespace atlas
{
	struct Ray;

	namespace rendering
	{
		class Session
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

			struct Slave
			{
				std::mutex mutex;
				uint8_t renderingBufferIdx = 0;
				Region region[2];
				Buffer buffer[2];

				bool hasBeenCollected = true;

				Slave() = default;
				Slave(const Slave &ref)
				{
					hasBeenCollected = ref.hasBeenCollected;
					renderingBufferIdx = ref.renderingBufferIdx;
				}
			};

			struct Report
			{
				Chronometer accBuildTime;
				Chronometer renderingTime;
			};

			ATLAS Session(const Scene &scene, uint32_t width, uint32_t height, uint32_t nbrSample);
			ATLAS ~Session();

			ATLAS void launch(const Camera &camera, Buffer &out);
			ATLAS void launch(const Camera &camera, int nbrThread);

			ATLAS void renderRegion(const Camera &camera, const Region &region, Buffer &dst);

			ATLAS void fetchResult(Buffer &dst);

			void processJobs(const Camera &camera, Slave &slave);

			inline operator bool() const { return (isRendering); }

			template <typename T = uint32_t>
			inline T getWidth() const { return (static_cast<T>(width)); }

			template <typename T = uint32_t>
			inline T getHeight() const { return (static_cast<T>(height)); }

			inline float getOutputImageRatio() const { return (static_cast<float>(width) / height); }
			inline uint32_t getSamples() const { return (nbrSample); }

			inline void setNear(float n) { tmin = n; }
			inline void setFar(float f) { tmax = f; }
			inline void setMaxDepth(int d) { maxDepth = d; }
			inline void setBlockSize(uint32_t size) { blockSize = size; }
			inline bool isWorking() const { return (isRendering); }
			inline float getNear() const { return (tmin); };
			inline float getFar() const { return (tmax); };
			inline int getMaxDepth() const { return (maxDepth); };
			inline uint32_t getBlockSize() const { return (blockSize); };

		private:
			std::atomic<bool> isRendering = false;
			
			Chronometer chrono;

			const Scene &scene;
			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t nbrSample = 0;

			std::vector<Acceleration> accelerations;

			std::atomic<uint8_t> nbrThreadWorking;
			std::mutex mutex;
			std::vector<std::thread> threads;
			std::vector<Slave> slaves;
			std::queue<Region> jobs;

			float tmin = 0.01f;
			float tmax = 100.f;
			int maxDepth = 124;
			uint32_t blockSize = 124;

			Report report;

			Texel traceRay(const Ray &ray, int depth = 0) const;

			void generateRenderingJobs();
		};
	}
}