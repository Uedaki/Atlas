#pragma once

#include <cstdint>
#include <vector>

#include "Shader.h"

namespace atlas
{
	class ShaderGroupBind;

	class ShaderGroup
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

		void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const
		{
			for (uint32_t i = shaders.size() - 1; i < shaders.size(); i--)
			{
				shaders[i]->evaluate(wo, si, sample, block);
			}
		}

	private:
		friend class ShaderGroupBind;
		uint32_t dataSize = 0;
		std::vector<Shader *> shaders;
		std::vector<ShadingOutput<uint8_t>> outputs;
	};

	class ShaderGroupBind : public Shader
	{
		virtual void registerOutputs(uint32_t &size) override
		{
			start = size;
			size += group->dataSize;
		}

		void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const override
		{
			DataBlock subBlock(block, start, group->dataSize);
			group->evaluate(wo, si, sample, subBlock);
		}

	private:
		uint32_t start;
		ShaderGroup *group;
	};
}