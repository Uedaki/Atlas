#pragma once

#include "atlas/core/Interaction.h"
#include "ShadingIO.h"
#include "atlas/core/Block.h"

namespace atlas
{
	struct Shader
	{
		virtual void registerOutputs(uint32_t &size) = 0;
		
		virtual void evaluate(const Vec3f &wo, const SurfaceInteraction &si, DataBlock &block) const = 0;
	};

	template <typename T>
	struct ConstantShader : public Shader
	{
		void registerOutputs(uint32_t &size) override
		{
			out.registerOutput(size);
		}

		void evaluate(const Vec3f &wo, const SurfaceInteraction &si, DataBlock &block) const override
		{
			out.set(block, value);
		}

		T value;
		ShadingOutput<T> out;
	};
}