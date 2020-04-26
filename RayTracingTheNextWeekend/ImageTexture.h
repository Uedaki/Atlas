#pragma once

#include "stb_image.h"

#include <string>

#include "Atlas/MaterialInput.h"
#include "atlas/Texture.h"
#include "Atlas/rendering/HitRecord.h"

class ImageTexture : public atlas::Texture
{
public:
	ImageTexture() = default;
	ImageTexture(const std::string &file)
	{
		data = stbi_load(file.c_str(), &nx, &ny, &channels, STBI_rgb);
	}
	ImageTexture(unsigned char *pixels, int a, int b) : data(pixels), nx(a), ny(b), channels(-1) {}

	~ImageTexture()
	{
		if (channels != -1)
			stbi_image_free(data);
	}

	glm::vec3 sample(const atlas::rendering::HitRecord &hit) const override
	{
		int i = hit.uv.x * nx;
		int j = (1 - hit.uv.y) * ny - 0.001;
		i = i < 0 ? 0 : i;
		j = j < 0 ? 0 : j;
		i = i > nx - 1 ? nx - 1 : i;
		j = j > ny - 1 ? ny - 1 : j;
		
		return (glm::vec3(static_cast<float>(data[3 * i + 3 * nx * j]) / 255.0f,
			static_cast<float>(data[3 * i + 3 * nx * j + 1]) / 255.0f,
			static_cast<float>(data[3 * i + 3 * nx * j + 2]) / 255.0f));
	}

private:
	unsigned char *data;
	int nx;
	int ny;
	int channels;
};