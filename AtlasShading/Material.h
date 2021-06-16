#pragma once

#include "Atlas/core/Interaction.h"
#include "atlas/core/Vectors.h"

#include "atlas/core/Telemetry.h"

#include "ShadingIO.h"
#include "BSDF.h"
#include "Shader.h"
#include "BSDFShader.h"
#include "atlas/core/Block.h"

namespace atlas
{
	class Material
	{
	public:
		template <typename T>
		inline T &addShader()
		{
			T *ptr = new T;
			ptr->registerOutputs(dataSize);
			shaders.push_back(ptr);
			return (*ptr);
		}

		template <typename T>
		inline ConstantShader<T> &addConstant(const T &value = T())
		{
			ConstantShader<T> &shader = addShader<ConstantShader<T>>();
			shader.value = value;
			return (shader);
		}

		BSDF sample(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample) const
		{
			BSDF bsdf;
			{
				DataBlock block(dataSize);
				for (size_t i = shaders.size() - 1; i < shaders.size(); i--)
				{
					shaders[i]->evaluate(wo, si, sample, block);
				}
				bsdf = bsdfInput.get(block);
			}
			return (bsdf);
		}

		void sample(const Block<Vec3f> &negWoWorld, const Block<SurfaceInteraction> &sis, const Block<Point2f> &samples, Block<BSDF> &bsdfs)
		{
			std::vector<Normal> ns(bsdfs.size());
			std::vector<Vec3f> ss(bsdfs.size());
			std::vector<Vec3f> ts(bsdfs.size());
			std::vector<Vec3f> wo(bsdfs.size());
			for (uint32_t i = 0; i < bsdfs.size(); i++)
			{
				ns[i] = sis[i].shading.n;
				ss[i] = normalize(sis[i].shading.dpdu);
				ts[i] = cross(sis[i].shading.n, ss[i]);
			}

			// Transform wo from world to local
			for (uint32_t i = 0; i < bsdfs.size(); i++)
				wo[i] = Vec3f(dot(-negWoWorld[i], ss[i]), dot(-negWoWorld[i], ts[i]), dot(-negWoWorld[i], ns[i]));

			{
				DataBlock block(dataSize);
				for (uint32_t i = 0; i < bsdfs.size(); i++)
				{
					for (size_t i = shaders.size() - 1; i < shaders.size(); i--)
					{
						shaders[i]->evaluate(wo[i], sis[i], samples[i], block);
					}
					bsdfs[i] = bsdfInput.get(block);
				}
			}

			// transform wi from local to world
			for (uint32_t i = 0; i < bsdfs.size(); i++)
				bsdfs[i].wi = Vec3f(ss[i].x * bsdfs[i].wi.x + ts[i].x * bsdfs[i].wi.y + ns[i].x * bsdfs[i].wi.z,
					ss[i].y * bsdfs[i].wi.x + ts[i].y * bsdfs[i].wi.y + ns[i].y * bsdfs[i].wi.z,
					ss[i].z * bsdfs[i].wi.x + ts[i].z * bsdfs[i].wi.y + ns[i].z * bsdfs[i].wi.z);
		}

		void bind(const BSDFShader &shader)
		{
			bsdfInput.bind(shader.out);
		}

	private:
		uint32_t dataSize = 0;
		ShadingInput<BSDF> bsdfInput;
		std::vector<Shader *> shaders;

		// TODO support for bump
	};
}