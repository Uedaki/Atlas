#pragma once

#include "Atlas/core/Interaction.h"
#include "atlas/core/Vectors.h"

#include "ShadingIO.h"
#include "BSDF.h"
#include "Shader.h"
#include "BSDFShader.h"
#include "DataBlock.h"

namespace atlas
{
	namespace sh
	{
		class Material
		{
		public:
			template <typename T>
			T &addShader()
			{
				T *ptr = new T;
				ptr->registerOutputs(dataSize);
				shaders.push_back(ptr);
				return (*ptr);
			}

			BSDF sample(const Vec3f &woWorld, const SurfaceInteraction &si, const Point2f &sample) const
			{
				BSDF bsdf;

				const Normal ns = si.shading.n;
				const Vec3f ss = normalize(si.shading.dpdu);
				const Vec3f ts = cross(ns, ss);

				// Transform wo from world to local
				Vec3f wo = Vec3f(dot(woWorld, ss), dot(woWorld, ts), dot(woWorld, ns));

				{
					DataBlock block(dataSize);
					for (uint32_t i = shaders.size() - 1; i < shaders.size(); i--)
					{
						shaders[i]->evaluate(wo, si, sample, block);
					}
					bsdf = bsdfInput.get(block);
				}

				// transform wi from local to world
				bsdf.wi = Vec3f(ss.x * bsdf.wi.x + ts.x * bsdf.wi.y + ns.x * bsdf.wi.z,
					ss.y * bsdf.wi.x + ts.y * bsdf.wi.y + ns.y * bsdf.wi.z,
					ss.z * bsdf.wi.x + ts.z * bsdf.wi.y + ns.z * bsdf.wi.z);

				return (bsdf);
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
}