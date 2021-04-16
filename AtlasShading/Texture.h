#pragma once

#include "AtlasShadingLibHeader.h"
#include "Shader.h"

#include "atlas/core/Random.h"

namespace atlas
{
	namespace sh
	{
		class Material;

		struct Texture : public Shader
		{
			void registerOutputs(uint32_t &size) override
			{
				color.registerOutput(size);
				alpha.registerOutput(size);
			}

			ShadingOutput<Spectrum> color;
			ShadingOutput<Float> alpha;
		};

		struct CheckerTexture : public Texture
		{
			void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const override
			{
				Float scale = iScale.get(block);
				Float sines = sin(si.p.x * scale) * sin(si.p.y * scale) * sin(si.p.z * scale);
				color.set(block, sines < 0 ? iColor1.get(block) : iColor2.get(block));
				alpha.set(block, 1);
			}

			ShadingInput<Spectrum> iColor1;
			ShadingInput<Spectrum> iColor2;
			ShadingInput<Float> iScale;
		};

		struct NoiseTexture : public Texture
		{
			Float noise(const Point3f &p) const
			{
				auto u = p.x - std::floor(p.x);
				auto v = p.y - std::floor(p.y);
				auto w = p.z - std::floor(p.z);

				auto i = static_cast<int>(std::floor(p.x));
				auto j = static_cast<int>(std::floor(p.y));
				auto k = static_cast<int>(std::floor(p.z));

				Vec3 c[2][2][2];
				for (uint8_t di = 0; di < 2; di++)
				{
					for (uint8_t dj = 0; dj < 2; dj++)
					{
						for (uint8_t dk = 0; dk < 2; dk++)
						{
							c[di][dj][dk] = ranVec[permX[(i + di) & 255] ^ permY[(j + dj) & 255] ^ permZ[(k + dk) & 255]];
						}
					}
				}

				return (perlinInterpolation(c, u, v, w));
			}

			void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const override
			{
				Float scale = iScale.get(block);
				Point3f p = scale * si.p;
				Float value = 0.5 * (1 + noise(p));
				color.set(block, Spectrum(value));
				alpha.set(block, value);
			}

			ShadingInput<Float> iScale;

			NoiseTexture()
			{
				ranVec = new Vec3[PointCount];
				for (uint32_t i = 0; i < PointCount; i++)
					ranVec[i] = 2.f * Vec3(random(), random(), random()) - Vec3(1);

				permX = generatePerm();
				permY = generatePerm();
				permZ = generatePerm();
			}

			~NoiseTexture()
			{
				delete[] ranVec;
				delete[] permX;
				delete[] permY;
				delete[] permZ;
			}

		private:
			static constexpr uint32_t PointCount = 256;
			Vec3 *ranVec;
			int *permX;
			int *permY;
			int *permZ;

			static int *generatePerm()
			{
				auto p = new int[PointCount];
				for (uint32_t i = 0; i < PointCount; i++)
					p[i] = i;

				permute(p, PointCount);

				return (p);
			}

			static void permute(int *p, int n)
			{
				for (int i = n - 1; i >= 0; i--)
				{
					int target = (int)(random() * (float)i);
					int tmp = p[i];
					p[i] = p[target];
					p[target] = tmp;
				}
			}

			static Float perlinInterpolation(Vec3 c[2][2][2], Float u, Float v, Float w)
			{
				Float acc = 0;
				auto uu = u * u * (3 - 2 * u);
				auto vv = v * v * (3 - 2 * v);
				auto ww = w * w * (3 - 2 * w);
				for (uint8_t i = 0; i < 2; i++)
				{
					for (uint8_t j = 0; j < 2; j++)
					{
						for (uint8_t k = 0; k < 2; k++)
						{
							Vec3 weight(u - i, v - j, w - k);
							acc += (i * uu + (1 - i) * (1 - uu))
								* (j * vv + (1 - j) * (1 - vv))
								* (k * ww + (1 - k) * (1 - ww))
								* dot(c[i][j][k], weight);
						}
					}
				}
				return (acc);
			}
		};

		struct TurbulenceNoiseTexture : public NoiseTexture
		{
			void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const override
			{
				Float scale = iScale.get(block);
				uint32_t depth = iDepth.get(block);
				Point3f p = si.p * scale;

				Float acc = 0;
				Float weight = 1;
				for (uint32_t i = 0; i < depth; i++)
				{
					acc += weight * noise(p);
					weight *= 0.5;
					p *= 2;
				}

				Float value = 0.5 * (1 + sin(scale * si.p.z + 10 * fabs(acc)));
				color.set(block, Spectrum(value));
				alpha.set(block, value);
			}

			ShadingInput<uint32_t> iDepth;
		};

		struct ImageTexture : public Texture
		{

		};
	}
}