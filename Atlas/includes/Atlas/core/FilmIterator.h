#pragma once

#include <cstdint>

#include "atlas/Atlas.h"
#include "atlas/core/Points.h"
#include "atlas/core/Bounds.h"
#include "atlas/core/Film.h"
#include "atlas/core/ImageIO.h"
#include "Atlas/AtlasLibHeader.h"

namespace atlas
{
	class FilmIterator
	{
	public:
		ATLAS FilmIterator(Film::Pixel *pixels, uint32_t size);

		ATLAS void start(uint32_t newSampleCount);
		ATLAS void clear(uint32_t sampleCount);

		ATLAS void save();
		ATLAS void accumulate();

		inline void addSample(const uint32_t pixelID, const Spectrum &LIn, Float sampleWeight = 1.f)
		{
			pixels[pixelID].color += LIn * sampleWeight;
		}

		inline uint32_t getSampleCount() const
		{
			return (sampleCount);
		}

		inline const Film::Pixel &getPixel(uint32_t pixelID) const
		{
			return (pixels[pixelID]);
		}

	private:
		uint32_t itCount = 0;
		uint32_t sampleCount = 0;
		const uint32_t size = 0;
		Film::Pixel *pixels = nullptr;
	};
}