#include "pch.h"
#include "Chronometer.h"

std::ostream &operator<<(std::ostream &os, const atlas::Chronometer &chronometer)
{
	uint32_t total = chronometer.elapsed<atlas::Chronometer::milliseconds>();;
	uint32_t hours = total / 60 / 60 / 1000;
	uint32_t minutes = (total / 60 / 1000) % 60;
	uint32_t seconds = (total / 1000) % 60;
	uint32_t milliseconds = total % 1000;

	if (hours != 0)
		os << hours << " hours ";
	if (hours != 0 || minutes != 0)
		os << minutes << " minutes ";
	if (hours != 0 || minutes != 0 || seconds != 0)
		os << seconds << " seconds";
	if (hours == 0 && minutes == 0)
	{
		if (seconds != 0)
			os << " ";
		os << milliseconds << " milliseconds";
	}

	return (os);
}