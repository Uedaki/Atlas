#pragma once

# ifdef OIDN_EXPORT
#   define OIDN  __declspec( dllexport )
# else
#   define OIDN __declspec( dllimport )
# endif

#include <glm/glm.hpp>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "Atlas/Buffer.h"
#include "Atlas/Chronometer.h"

namespace atlas
{
	namespace ext
	{
		class Oidn
		{
		public:
			OIDN Oidn(size_t width, size_t height);

			OIDN void launch(Buffer &buffer);
			OIDN void asyncLaunch(Buffer &buffer);

			inline const std::vector<glm::vec3> &getDenoisedOutput() const { return (outputImage); };

		private:
			std::atomic<bool> inProgress = false;
			size_t width;
			size_t height;
			std::vector<glm::vec3> outputImage;
			std::unique_ptr<std::thread> thread;
			Chronometer denoisingTime;
		};
	}
}