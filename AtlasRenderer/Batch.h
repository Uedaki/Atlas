#pragma once

#include <vector>

#include "Atlas/Atlas.h"
#include "Atlas/core/Points.h"
#include "Atlas/core/RgbSpectrum.h"
#include "Atlas/core/Vectors.h"

namespace atlas
{
	struct Batch
	{
		std::vector<Point3f> origins;
		std::vector<Vec3f> directions;
		std::vector<Spectrum> colors;
		std::vector<uint32_t> pixelIDs;
		std::vector<uint16_t> sampleIDs;
		std::vector<uint16_t> depths;
		std::vector<Float> tNears;

		uint32_t size() const
		{
			return (static_cast<uint32_t>(origins.size()));
		}

		void resize(size_t size)
		{
			origins.resize(size);
			directions.resize(size);
			colors.resize(size);
			pixelIDs.resize(size);
			sampleIDs.resize(size);
			depths.resize(size);
			tNears.resize(size);
		}

		void swap(uint32_t a, uint32_t b)
		{
			std::swap(origins[a], origins[b]);
			std::swap(directions[a], directions[b]);
			std::swap(colors[a], colors[b]);
			std::swap(pixelIDs[a], pixelIDs[b]);
			std::swap(sampleIDs[a], sampleIDs[b]);
			std::swap(depths[a], depths[b]);
			std::swap(tNears[a], tNears[b]);
		}
	};
}