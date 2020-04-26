#pragma once

# ifdef ATLAS_EXPORT
#   define ATLAS  __declspec( dllexport )
# else
#   define ATLAS __declspec( dllimport )
# endif

#include <chrono>
#include <ostream>

namespace atlas
{
	class Chronometer
	{
	public:
		typedef std::chrono::hours hours;
		typedef std::chrono::minutes minutes;
		typedef std::chrono::seconds seconds;
		typedef std::chrono::milliseconds milliseconds;

		inline void start()
		{
			startPoint = std::chrono::system_clock::now();
		}

		void stop()
		{
			endPoint = std::chrono::system_clock::now();
		}

		template <typename T>
		inline uint32_t elapsed() const
		{
			return (static_cast<uint32_t>(std::chrono::duration_cast<T>(endPoint - startPoint).count()));
		}

	private:
		std::chrono::time_point<std::chrono::system_clock> startPoint;
		std::chrono::time_point<std::chrono::system_clock> endPoint;
	};
}

ATLAS std::ostream &operator<<(std::ostream &os, const atlas::Chronometer &c);