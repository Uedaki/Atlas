#pragma once

#include <vector>

#include "Atlas/Atlas.h"
#include "Atlas/core/Points.h"
#include "Atlas/core/RgbSpectrum.h"
#include "Atlas/core/Vectors.h"

#include "Atlas/core/simd/Simd.h"
#include "Atlas/core/simd/SPoints.h"
#include "Atlas/core/simd/SRgbSpectrum.h"
#include "Atlas/core/simd/SVectors.h"

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

	struct S4Batch
	{
		std::vector<S4Point3> origins;
		std::vector<S4Vec3> directions;
		std::vector<S4RgbSpectrum> colors;
		std::vector<S4Int> pixelIDs;
		std::vector<S4Int> sampleIDs;
		std::vector<S4Int> depths;
		std::vector<S4Int> tNears;

		uint32_t size() const
		{
			return (static_cast<uint32_t>(origins.size()));
		}

		void resize(size_t size)
		{
			size_t newSize = (size + 3) + 4;

			origins.resize(newSize);
			directions.resize(newSize);
			colors.resize(newSize);
			pixelIDs.resize(newSize);
			sampleIDs.resize(newSize);
			depths.resize(newSize);
			tNears.resize(newSize);
		}

		void swap(uint32_t a, uint32_t b, S4Bool mask)
		{
			atlas::swap(origins[a], origins[b], mask);
			atlas::swap(directions[a], directions[b], mask);
			atlas::swap(colors[a], colors[b], mask);
			atlas::swap(pixelIDs[a], pixelIDs[b], mask);
			atlas::swap(sampleIDs[a], sampleIDs[b], mask);
			atlas::swap(depths[a], depths[b], mask);
			atlas::swap(tNears[a], tNears[b], mask);
		}
	};
}