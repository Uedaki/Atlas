#pragma once

#include <cstdint>

#include "atlas/Atlas.h"
#include "atlas/core/Points.h"
#include "atlas/core/Bounds.h"
#include "atlas/core/ImageIO.h"

namespace atlas
{
	class IterationFilm
	{
	public:
		IterationFilm(const Point2i &resolution, const Bounds2f &cropWindow, Float diagonal)
			: resolution(resolution)
			, croppedPixelBounds(Point2i(std::ceil(resolution.x * cropWindow.min.x), std::ceil(resolution.y * cropWindow.min.y)),
				Point2i(std::ceil(resolution.x * cropWindow.max.x), std::ceil(resolution.y * cropWindow.max.y)))
			, diagonal(diagonal)
		{
			pixels = std::unique_ptr<Spectrum[]>(new Spectrum[croppedPixelBounds.surfaceArea()]);
			clear();
		}

		void startIteration(uint32_t sampleCount)
		{
			if (itCount != 0)
				writeImage();
			itCount++;
			invSampleCount = 1.0 / (Float)sampleCount;
		}

		void clear()
		{
			uint32_t size = croppedPixelBounds.surfaceArea();
			for (uint32_t i = 0; i < size; i++)
			{
				pixels[i] = BLACK;
			}
		}

		Bounds2i getSampleBounds() const
		{
			Bounds2f bounds(floor(Point2f(croppedPixelBounds.min) + Vec2f(0.5f, 0.5f) + Vec2f(0.f, 0.f)),
				ceil(Point2f(croppedPixelBounds.max) + Vec2f(0.5f, 0.5f) + Vec2f(0.f, 0.f)));
			return (static_cast<Bounds2i>(bounds));
		}

		Bounds2f getPhysicalExtent() const
		{
			Float aspect = (Float)resolution.y / (Float)resolution.x;
			Float x = std::sqrt(diagonal * diagonal / (1 + aspect * aspect));
			Float y = aspect * x;
			return Bounds2f(Point2f(-x / 2, -y / 2), Point2f(x / 2, y / 2));
		}

		inline void addSample(const uint32_t pixelID, const Spectrum &LIn, Float sampleWeight = 1.f)
		{
			pixels[pixelID] += LIn * sampleWeight;
		}

		void writeImage()
		{
			std::string filename = "iteration-" + std::to_string(itCount) + ".ppm";
			for (uint32_t i = 0; i < croppedPixelBounds.surfaceArea(); i++)
			{
				pixels[i].r = std::sqrt(pixels[i].r * invSampleCount);
				pixels[i].g = std::sqrt(pixels[i].g * invSampleCount);
				pixels[i].b = std::sqrt(pixels[i].b * invSampleCount);
			}

			writeImageToFile(filename, (Float*)pixels.get(), croppedPixelBounds, resolution);
		}

		const Point2i resolution;
		const Bounds2i croppedPixelBounds;
		const Float diagonal;

	private:
		uint32_t itCount = 0;
		Float invSampleCount = 0;
		std::unique_ptr<Spectrum[]> pixels;
	};
}