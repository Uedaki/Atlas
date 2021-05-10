#pragma once

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>
#include <ostream>

#include "Atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"

class Telemetry
{
public:
	struct SectorTime
	{
		uint32_t count = 0;
		Float t = 0;
		Float min = std::numeric_limits<Float>::max();
		Float max = std::numeric_limits<Float>::min();

		SectorTime(const std::string &path)
		{
			Telemetry::getInstance().registerSector(path, this);
		}
	};

	class SingleTimeScope
	{
	public:
		SingleTimeScope(SectorTime &sector)
			: sector(sector), start(std::chrono::high_resolution_clock::now())
		{}

		~SingleTimeScope()
		{
			sector.t = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count());
		}

	private:
		SectorTime &sector;
		std::chrono::time_point<std::chrono::high_resolution_clock> start;
	};

	class MulTimeScope
	{
	public:
		MulTimeScope(SectorTime &sector)
			: sector(sector), start(std::chrono::high_resolution_clock::now()), bIsClosed(false)
		{}

		~MulTimeScope()
		{
			if (bIsClosed)
				return;
			float val = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count());
			sector.t = (sector.t * sector.count + val) / (sector.count + 1);
			sector.max = std::max(sector.max, val);
			sector.min = std::min(sector.min, val);
			sector.count += 1;
		}

		void stop()
		{
			float val = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count());
			sector.t = (sector.t * sector.count + val) / (sector.count + 1);
			sector.max = std::max(sector.max, val);
			sector.min = std::min(sector.min, val);
			sector.count += 1;
			bIsClosed = true;
		}

	private:
		SectorTime &sector;
		std::chrono::time_point<std::chrono::high_resolution_clock> start;
		bool bIsClosed;
	};

	ATLAS static void report();

private:
	friend struct SectorTime;

	ATLAS static Telemetry instance;

	inline static Telemetry &getInstance()
	{
		return (instance);
	}

	inline void registerSector(const std::string &path, const SectorTime *sector)
	{
		sectors.emplace_back();
		sectors.back().first = path;
		sectors.back().second = sector;
	}

	std::vector<std::pair<std::string, const SectorTime *>> sectors;
};

//#define TELEMETRY(name, path) static Telemetry::SectorTime __tlm_st_##name(path); Telemetry::SingleTimeScope __tlm_stm_##name(__tlm_st_##name);
#define TELEMETRY(name, path) static Telemetry::SectorTime __tlm_st_##name(path); Telemetry::MulTimeScope __tlm_mtm_##name(__tlm_st_##name);
#define CLOSE_TELEMTRY(name) __tlm_mtm_##name.stop()
#define PRINT_TELEMETRY_REPORT() Telemetry::report()