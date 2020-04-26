#pragma once

#include <glm/glm.hpp>

#include "Atlas/Texture.h"
#include "Atlas/Tools.h"

class NoiseTexture : public atlas::Texture 
{
public:

	glm::vec3 sample(const atlas::rendering::HitRecord &hit) const override
	{
		float u = hit.p.x - glm::floor(hit.p.x);
		float v = hit.p.y - glm::floor(hit.p.y);
		float w = hit.p.z - glm::floor(hit.p.z);
		int i = static_cast<int>(glm::floor(hit.p.x));
		int j = static_cast<int>(glm::floor(hit.p.y));
		int k = static_cast<int>(glm::floor(hit.p.z));
		glm::vec3 c[2][2][2];
		for (int di = 0; di < 2; di++)
			for (int dj = 0; dj < 2; dj++)
				for (int dk = 0; dk < 2; dk++)
					c[di][dj][dk] = noise.ranVec[noise.permX[(i + di) & 255] ^ noise.permY[(j + dj) & 255] ^ noise.permZ[(k + dk) & 255]];
		return (glm::vec3(1) * trilinearInterpolation(c, u, v, w));
	}

private:
	struct Noise
	{
		glm::vec3 ranVec[256];
		int permX[256];
		int permY[256];
		int permZ[256];

		Noise()
		{
			generatePerlin(ranVec);
			generatePermPerlin(permX);
			generatePermPerlin(permY);
			generatePermPerlin(permZ);
		}

		static void generatePerlin(glm::vec3 *p)
		{
			for (int i = 0; i < 256; ++i)
				p[i] = glm::normalize(glm::vec3(-1 + 2 * atlas::Tools::rand(), -1 + 2 * atlas::Tools::rand(), -1 + 2 * atlas::Tools::rand()));
		}

		static void permute(int *p, int n)
		{
			for (int i = n - 1; i > 0; i--)
			{
				int target = static_cast<int>(atlas::Tools::rand() * (i + 1));
				int tmp = p[i];
				p[i] = p[target];
				p[target] = tmp;
			}
		}

		static void generatePermPerlin(int *p)
		{
			for (int i = 0; i < 256; i++)
				p[i] = i;
			permute(p, 256);
		}
	};

	static Noise noise;

	inline float trilinearInterpolation(glm::vec3 c[2][2][2], float u, float v, float w) const
	{
		float uu = u * u * (3 - 2 * u);
		float vv = v * v * (3 - 2 * v);
		float ww = w * w * (3 - 2 * w);
		float accum = 0;
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				for (int k = 0; k < 2; k++)
				{
					glm::vec3 weight(u - i, v - j, w - k);
					accum += (i * uu + (1 - i) * (1 - uu))
						* (j * vv + (1 - j) * (1 - vv))
						* (k * ww + (1 - k) * (1 - ww))
						* glm::dot(c[i][j][k], weight);
				}
		return (accum);
	}
};