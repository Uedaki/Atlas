#pragma once

#include "atlas/core/Reflection.h"
#include "atlas/core/Sampling.h"

#include "BSDF.h"
#include "Shader.h"
#include "ShadingIO.h"
#include "Onb.h"

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
				Onb uvw(si.n);
				
				bsdf.wi = uvw.local(cosineSampleDirection(sample));
				if (wo.z < 0)
				{
					bsdf.wi.z *= -1;
				}
				bsdf.pdf = dot(uvw.w, bsdf.wi) * INV_PI;
				bsdf.scatteringPdf = scatteringPdf(si, wo, bsdf.wi);
				bsdf.Li = f(wo, bsdf.wi, block);
				out.set(block, bsdf);
			}

			virtual Spectrum f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const
			{
				return (BLACK);
			}

			virtual Float scatteringPdf(const Interaction &intr, const Vec3f &wo, const Vec3f &wi) const
			{
				Float cosine = dot(intr.n, wi);
				return (cosine < 0 ? 0 : cosine * INV_PI);
			}

			ShadingOutput<BSDF> out;
		};
	}
}