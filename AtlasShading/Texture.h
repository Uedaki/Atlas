#pragma once

#include "AtlasShadingLibHeader.h"
#include "Shader.h"

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
			void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, std::vector<uint8_t> &data) const override
			{
				Float tmp = scale.getValue(data);
				Float sines = sin(si.p.x * tmp) * sin(si.p.y * tmp) * sin(si.p.z * tmp);
				color.set(data, sines < 0 ? color1.getValue(data) : color2.getValue(data));
				alpha.set(data, 1);
			}

			ShadingInput<Spectrum> color1;
			ShadingInput<Spectrum> color2;
			ShadingInput<Float> scale;
		};

		struct NoiseTexture : public Texture
		{

		};

		struct ImageTexture : public Texture
		{

		};
	}
}