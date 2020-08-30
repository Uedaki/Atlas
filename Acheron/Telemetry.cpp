#include "pch.h"
#include "Telemetry.h"

#ifndef _DEBUG

#include <iostream>

Telemetry Telemetry::instance;

void Telemetry::SectorTime::reset()
{
	count = 0;
	t = 0;
	min = std::numeric_limits<float>::max();
	max = std::numeric_limits<float>::min();
}

Telemetry::SectorID Telemetry::getSectorID(const std::string &path)
{
	for (uint32_t i = 0; i < instance.paths.size(); i++)
	{
		if (instance.paths[i] == path)
			return (i);
	}
	instance.paths.push_back(path);
	instance.sectors.emplace_back();
	return (static_cast<SectorID>(instance.paths.size() - 1));
}

std::ostream &operator<<(std::ostream &os, const Telemetry::SectorTime &s)
{
	if (s.count == 0)
	{
		float val = s.t;
		float newVal = 0;
		std::string ext = "ms";
		if ((newVal = val / 1000) > 1.f)
		{
			val = newVal;
			ext = "s";
			if ((newVal = val / 60) > 1.f)
			{
				val = newVal;
				ext = "min";
				if ((newVal = val / 60) > 1.f)
				{
					val = newVal;
					ext = "h";
				}
			}
		}

		os << val << ext;
	}
	else
	{
		float div = 1;
		float newDiv = 1;
		std::string ext = "ms";
		newDiv /= 1000;
		if (s.t * newDiv > 1.f)
		{
			div = newDiv;
			newDiv /= 60;
			ext = "s";
			if (s.t * newDiv > 1.f)
			{
				div = newDiv;
				ext = "min";
				newDiv /= 60;
				if (s.t * newDiv > 1.f)
				{
					div = newDiv;
					ext = "h";
				}
			}
		}

		os << s.t * div << ext << " avg (min: " << s.min * div << ext << " max:" << s.max * div << ext << ")";
	}
	return (os);
}

void Telemetry::printReport()
{
	std::cout << "Telemetry report:" << "\n";
	for (uint32_t i = 0; i < instance.paths.size(); i++)
	{
		std::cout << instance.paths[i] << ": " << instance.sectors[i] << "\n";
	}
	std::cout << std::endl;
}

#endif