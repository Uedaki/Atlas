#pragma once

#ifndef _DEBUG

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

# ifdef ACHERON_EXPORTS
#   define ACH  __declspec( dllexport )
# else
#   define ACH __declspec( dllimport )
# endif

class Telemetry
{
public:
	typedef uint32_t SectorID;

	struct SectorTime
	{
		uint32_t count = 0;
		float t = 0;
		float min = std::numeric_limits<float>::max();
		float max = std::numeric_limits<float>::min();

		void reset();
	};

	class SingleTimedScope
	{
	public:
		SingleTimedScope(SectorID sectorID)
			: id(sectorID), start(std::chrono::high_resolution_clock::now())
		{}

		~SingleTimedScope()
		{
			instance.sectors[id].t = (float)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
		}

	private:
		SectorID id;
		std::chrono::time_point<std::chrono::high_resolution_clock> start;
	};

	class MultipleTimedScope
	{
	public:
		MultipleTimedScope(SectorID sectorID)
			: id(sectorID), start(std::chrono::high_resolution_clock::now())
		{}

		~MultipleTimedScope()
		{
			float val = (float)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
			instance.sectors[id].t = (instance.sectors[id].t * instance.sectors[id].count + val) / (instance.sectors[id].count + 1);
			instance.sectors[id].min = std::min(instance.sectors[id].min, val);
			instance.sectors[id].max = std::max(instance.sectors[id].max, val);
			instance.sectors[id].count += 1;
		}

	private:
		SectorID id;
		std::chrono::time_point<std::chrono::high_resolution_clock> start;
	};

	ACH static SectorID getSectorID(const std::string &path);
	ACH static void printReport();

private:
	static Telemetry instance;
	std::vector<std::string> paths;
	std::vector<SectorTime> sectors;
};

#define DEFINE_SECTOR(name, path) static Telemetry::SectorID name = Telemetry::getSectorID(path)

#define SINGLE_TIMED_SCOPE(id) Telemetry::SingleTimedScope __sts(id)
#define MULTIPLE_TIMED_SCOPE(id) Telemetry::MultipleTimedScope __mts(id)

#define PRINT_TELEMETRY_REPORT() Telemetry::printReport()

#else

#define DEFINE_SECTOR(name, path)

#define SINGLE_TIMED_SCOPE(id)
#define MULTIPLE_TIMED_SCOPE(id)

#define PRINT_TELEMETRY_REPORT()

#endif