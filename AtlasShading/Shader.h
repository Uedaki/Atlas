#pragma once

#include "atlas/core/Interaction.h"
#include "ShadingIO.h"

namespace atlas
{
	namespace sh
	{
		struct Shader
		{
			virtual void registerOutputs(uint32_t &size) {}
			virtual void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, std::vector<uint8_t> &data) const = 0;
		};

		template <typename T>
		struct ConstantShader : public Shader
		{
			void registerOutputs(uint32_t &size) override
			{
				out.registerOutput(size);
			}

			void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, std::vector<uint8_t> &data) const override
			{
				out.set(data, value);
			}

			T value;
			ShadingOutput<T> out;
		};
	}
}