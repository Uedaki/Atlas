#pragma once

#include "Atlas/core/Primitive.h"

#include "atlas/core/Batch.h"
#include "atlas/core/Payload.h"
#include "ThreadPool.h"

#include <iostream>

namespace atlas
{
	namespace task
	{
		class TraceRays : public ThreadedTask
		{
		public:
			static constexpr uint32_t maxConeSize = 64;
			static constexpr uint32_t maxPackSize = maxConeSize * 4;

			struct Data
			{
				Batch *batch;
				const Primitive *scene;

				Block<SurfaceInteraction> *interactions;
			};

			TraceRays(Data &data)
				: data(data)
			{}

			bool preExecute() override;
			void execute() override;
			void postExecute() override;

			BoundingCone packRaysInCone(uint32_t startingIndex, uint32_t size);

		private:
			Data data;

			std::vector<Float> tmax;

			std::atomic<uint32_t> traceRaysIndex;
		};
	}
}