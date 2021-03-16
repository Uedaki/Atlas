#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Bounds.h"
#include "atlas/core/Filter.h"
#include "atlas/core/Points.h"
#include "atlas/core/RgbSpectrum.h"

namespace atlas
{
	class Film
	{
	public:
		struct Info
		{
			atlas::Point2i resolution = Point2i(1280, 720);
			atlas::Bounds2f cropWindow = Bounds2f(Point2f(0, 0), Point2f(1, 1));
			Filter *filter = nullptr;
			Float diagonal = 35;
			std::string filename;
			Float scale = 1;
			Float maxSampleLuminance = INFINITY;
		};

		struct Pixel
		{
			Spectrum color = { 0, 0, 0 };
			Float filterWeightSum = 0;
		};

		ATLAS Film(const Info &info);

		ATLAS Bounds2i getSampleBounds() const;
		ATLAS Bounds2f getPhysicalExtent() const;

		ATLAS void addSample(const Point2i &pos, const Point2f &pFilm, const Spectrum &L, Float sampleWeight = 1.f);

		ATLAS Pixel &getPixel(const Point2i &p);
		ATLAS const Pixel &getPixel(const Point2i &p) const;

		ATLAS void writeImage(Float splatScale = 1);
		ATLAS void clear();

		const Point2i resolution;
		const Bounds2i croppedPixelBounds;
		const Float diagonal;
		const std::string filename;
		const Float scale;

	private:
		std::unique_ptr<Pixel[]> pixels;
		std::unique_ptr<Filter> filter;

		static constexpr int filterTableWidth = 16;
		Float filterTable[filterTableWidth * filterTableWidth];

		Float maxSampleLuminance;
	};

	typedef Film::Info FilmInfo;
}