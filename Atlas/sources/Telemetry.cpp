#include "Atlas/core/Telemetry.h"

#include <iostream>

Telemetry Telemetry::instance;

std::ostream &operator<<(std::ostream &os, const Telemetry::SectorTime &s)
{
	float val = s.t;
	float div = 1;
	std::string ext = "ms";
	if (val / 1000 > 1.f)
	{
		val /= 1000;
		div /= 1000;
		ext = "s";

		if (val / 60 > 1.f)
		{
			val /= 60;
			div /= 60;
			ext = "min";
			if (val / 60 > 1.f)
			{
				val /= 60;
				div /= 60;
				ext = "h";
			}
		}
	}

	if (s.count == 0)
		os << val << ext;
	else
		os << "executed " << s.count << " avg " << val << ext << " (min: " << s.min * div << ext << " max: " << s.max * div << ext << " total: " << val * s.count << ext << ")";
	return (os);
}

void Telemetry::report()
{
	std::cout << "Telemetry report:" << "\n";
	for (auto &val : instance.sectors)
	{
		std::cout << val.first << " " << *val.second << std::endl;
	}
}