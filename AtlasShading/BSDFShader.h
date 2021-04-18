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

			void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const override
			{
				BSDF bsdf = {};
				bsdf.wi = cosineSampleHemisphere(sample);
				if (wo.z < 0)
					bsdf.wi.z *= -1;
				bsdf.pdf = pdf(wo, bsdf.wi);
				bsdf.Li = f(wo, bsdf.wi, block);
				out.set(block, bsdf);
			}

			virtual Spectrum f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const
			{
				return (BLACK);
			}

			virtual Float pdf(const Vec3f &wo, const Vec3f &wi) const
			{
				return sameHemisphere(wo, wi) ? absCosTheta(wi) * INV_PI : 0;
			}

			ShadingOutput<BSDF> out;
		};
	}
}