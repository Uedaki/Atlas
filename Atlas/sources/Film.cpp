#include "atlas/core/Film.h"

#include <memory>

#include "atlas/core/ImageIO.h"

using namespace atlas;

Film::Film(const Film::Info &info)
	: resolution(info.resolution)
	, croppedPixelBounds(Point2i(std::ceil(info.resolution.x *info.cropWindow.min.x), std::ceil(info.resolution.y *info.cropWindow.min.y)),
		Point2i(std::ceil(info.resolution.x *info.cropWindow.max.x), std::ceil(info.resolution.y *info.cropWindow.max.y)))
	, diagonal(info.diagonal)
    , scale(info.scale)
	, filename(info.filename)
    , filter(info.filter)
    , maxSampleLuminance(info.maxSampleLuminance)
{
    pixels = std::unique_ptr<Pixel[]>(new Pixel[croppedPixelBounds.surfaceArea()]);
    clear();

    int offset = 0;
    for (int y = 0; y < filterTableWidth; ++y) {
        for (int x = 0; x < filterTableWidth; ++x, ++offset) {
            Point2f p;
            p.x = (x + 0.5f) * filter->radius.x / filterTableWidth;
            p.y = (y + 0.5f) * filter->radius.y / filterTableWidth;
            filterTable[offset] = filter->evaluate(p);
        }
    }
}

Bounds2i Film::getSampleBounds() const
{
	Bounds2f bounds(floor(Point2f(croppedPixelBounds.min) + Vec2f(0.5f, 0.5f) + Vec2f(0.f, 0.f) /* filter->radius */),
		ceil(Point2f(croppedPixelBounds.max) + Vec2f(0.5f, 0.5f) + Vec2f(0.f, 0.f) /* filter->radius */));
	return (static_cast<Bounds2i>(bounds));
}

Bounds2f Film::getPhysicalExtent() const
{
	Float aspect = (Float)resolution.y / (Float)resolution.x;
	Float x = std::sqrt(diagonal * diagonal / (1 + aspect * aspect));
	Float y = aspect * x;
	return Bounds2f(Point2f(-x / 2, -y / 2), Point2f(x / 2, y / 2));
}

void Film::addSample(const Point2i &pos, const Point2f &pFilm, const Spectrum &LIn, Float sampleWeight)
{
    //Pixel &pixel = getPixel(pos);
    //pixel.color += LIn;
    //pixel.filterWeightSum += 1;

    Spectrum L = LIn;
    if (L.y() > maxSampleLuminance)
        L *= maxSampleLuminance / L.y();

    Point2f pFilmDiscrete = pFilm - Vec2f(0.5f, 0.5f);
    Point2i p0 = (Point2i)ceil(pFilmDiscrete - filter->radius);
    Point2i p1 = (Point2i)floor(pFilmDiscrete + filter->radius) + Point2i(1, 1);
    p0 = max(p0, croppedPixelBounds.min);
    p1 = min(p1, croppedPixelBounds.max);

    int *ifx = new int[p1.x - p0.x];
    for (int x = p0.x; x < p1.x; ++x)
    {
        Float fx = std::abs((x - pFilmDiscrete.x) * filter->invRadius.x * filterTableWidth);
        ifx[x - p0.x] = std::min((int)std::floor(fx), filterTableWidth - 1);
    }

    int *ify = new int[p1.y - p0.y];
    for (int y = p0.y; y < p1.y; ++y)
    {
        Float fy = std::abs((y - pFilmDiscrete.y) * filter->invRadius.y * filterTableWidth);
        ify[y - p0.y] = std::min((int)std::floor(fy), filterTableWidth - 1);
    }

    for (int y = p0.y; y < p1.y; ++y)
    {
        for (int x = p0.x; x < p1.x; ++x)
        {
            int offset = ify[y - p0.y] * filterTableWidth + ifx[x - p0.x];
            Float filterWeight = filterTable[offset];

            Pixel &pixel = getPixel(Point2i(x, y));
            pixel.color += L * sampleWeight * filterWeight;
            pixel.filterWeightSum += filterWeight;
        }
    }

    delete[] ifx;
    delete[] ify;
}

Film::Pixel &Film::getPixel(const Point2i &p)
{
    int width = croppedPixelBounds.max.x - croppedPixelBounds.min.x;
    int offset = (p.x - croppedPixelBounds.min.x) + (p.y - croppedPixelBounds.min.y) * width;
    return pixels[offset];
}

const Film::Pixel &Film::getPixel(const Point2i &p) const
{
    int width = croppedPixelBounds.max.x - croppedPixelBounds.min.x;
    int offset = (p.x - croppedPixelBounds.min.x) + (p.y - croppedPixelBounds.min.y) * width;
    return pixels[offset];
}

#include <iostream>

void Film::writeImage(Float splatScale)
{
    std::unique_ptr<Float[]> rgb(new Float[3 * croppedPixelBounds.surfaceArea()]);
    int offset = 0;
    for (Point2i p : croppedPixelBounds)
    {
        Pixel &pixel = getPixel(p);

        rgb[3 * offset] = pixel.color.r;
        rgb[3 * offset + 1] = pixel.color.g;
        rgb[3 * offset + 2] = pixel.color.b;

        Float filterWeightSum = pixel.filterWeightSum;
        if (filterWeightSum != 0)
        {
            Float invWt = (Float)1 / filterWeightSum;
            rgb[3 * offset] = std::max((Float)0, rgb[3 * offset] * invWt);
            rgb[3 * offset + 1] =
                std::max((Float)0, rgb[3 * offset + 1] * invWt);
            rgb[3 * offset + 2] =
                std::max((Float)0, rgb[3 * offset + 2] * invWt);
        }

        //Float splatRGB[3] = { pixel.color[0], pixel.color[1], pixel.color[2] };
        //rgb[3 * offset] += splatScale * splatRGB[0];
        //rgb[3 * offset + 1] += splatScale * splatRGB[1];
        //rgb[3 * offset + 2] += splatScale * splatRGB[2];

        // Scale pixel value by _scale_
        rgb[3 * offset] *= scale;
        rgb[3 * offset + 1] *= scale;
        rgb[3 * offset + 2] *= scale;
        ++offset;
    }

    writeImageToFile(filename, &rgb[0], croppedPixelBounds, resolution);
}

void Film::clear()
{
    for (Point2i p : croppedPixelBounds)
    {
        Pixel &pixel = getPixel(p);
        pixel.color = Spectrum(0);
        pixel.filterWeightSum = 0;
    }
}