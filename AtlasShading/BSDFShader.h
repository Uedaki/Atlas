#pragma once

#include "atlas/core/Reflection.h"
#include "atlas/core/Sampling.h"

#include "BSDF.h"
#include "Shader.h"
#include "ShadingIO.h"

namespace atlas
{
	namespace sh
	{
		struct BSDFShader : public Shader
		{
		public:
			void registerOutputs(uint32_t &size) override
			{
				out.registerOutput(size);
			}

			void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, std::vector<uint8_t> &data) const override
			{
				BSDF bsdf;
				bsdf.wi = cosineSampleHemisphere(sample);
				if (wo.z < 0)
					bsdf.wi.z *= -1;
				bsdf.pdf = pdf(wo, bsdf.wi);
				bsdf.color = f(wo, bsdf.wi, data);
				out.set(data, bsdf);
			}

			virtual Spectrum f(const Vec3f &wo, const Vec3f &wi, const std::vector<uint8_t> &data) const = 0;

			virtual Float pdf(const Vec3f &wo, const Vec3f &wi) const
			{
				return sameHemisphere(wo, wi) ? absCosTheta(wi) * INV_PI : 0;
			}

			ShadingOutput<BSDF> out;
		};
	}
}