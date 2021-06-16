#include "ImageLibrary.h"

atlas::ImageLibrary atlas::ImageLibrary::instance;

bool atlas::ImageWrapper::load()
{
	int nbComp;
	buffer = stbi_load(filename.c_str(), &width, &height, &nbComp, 4);
	if (buffer)
	{
		bIsLoaded = true;
		bHasAlpha = nbComp == 4;
		return (true);
	}
	return (false);
}

void atlas::ImageWrapper::unload()
{
	if (buffer)
		free(buffer);
	bIsLoaded = false;
}

atlas::Spectrum atlas::ImageWrapper::sample(const Point2f &uv, const uint32_t callValue, Float &alpha)
{
	uint32_t x = (uint32_t)(uv.x * width) % width;
	uint32_t y = (uint32_t)(uv.y * height) % height;
	uint8_t *rgba = &buffer[(x + y * width) * 4];
	if (bHasAlpha)
		alpha = rgba[3];
	else
		alpha = 1;
	lastCallValue = callValue;
	return (Spectrum((Float)rgba[0] / 255, (Float)rgba[1] / 255, (Float)rgba[2] / 255));
}

atlas::ImageID atlas::ImageLibrary::privateRequestID(const std::string &filename)
{
	for (uint32_t i = 0; i < images.size(); i++)
	{
		if (images[i].getFilename() == filename)
		{
			return (i);
		}
	}
	images.emplace_back(filename);
	return (images.size() - 1);
}

atlas::Spectrum atlas::ImageLibrary::privateSample(const ImageID id, const Point2f &uv, Float &alpha)
{
	if (!images[id].isLoaded())
	{
		// if need space, free the texture which spent the most time unused
		if (!images[id].load())
		{
			alpha = 1;
			return (Spectrum(1, 0, 0));
		}
	}
	Spectrum color = images[id].sample(uv, nextCallValue, alpha);
	nextCallValue++;
	return (color);
}