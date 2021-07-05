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

		BSDFSample sample(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample) const
		{
			BSDFSample bsdf;
			{
				DataBlock block(dataSize);
				for (size_t i = shaders.size() - 1; i > 0; i--)
				{
					shaders[i]->evaluate(wo, si, sample, block);
				}
				shaders[0]->evaluate(wo, si, sample, block);
				bsdf = bsdfInput.get(block);
			}
			return (bsdf);
		}

		void bind(const BSDFShader &shader)
		{
			bsdfInput.bind(shader.out);
		}

	private:
		uint32_t dataSize = 0;
		ShadingInput<BSDFSample> bsdfInput;
		std::vector<Shader *> shaders;

		// TODO support for bump
	};
}