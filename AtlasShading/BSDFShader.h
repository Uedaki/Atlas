#pragma once

#include "atlas/core/Reflection.h"
#include "atlas/core/Sampling.h"

#include "BSDF.h"
#include "Shader.h"
#include "ShadingIO.h"
#include "Onb.h"

namespace atlas
{
	struct BSDFShader : public Shader
	{
	public:
		void registerOutputs(uint32_t &size) override
		{
			out.registerOutput(size);
		}

		void evaluate(const Vec3f &wo, const Vec3f &wi, const SurfaceInteraction &si, DataBlock &block) const final
		{
			BSDFSample bsdf = {};
			evaluateBsdf(wo, bsdf.wi, si, block, bsdf);
			out.set(block, bsdf);
		}

		void evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const final
		{
			BSDFSample bsdf = {};

			evaluateWi(wo, si, sample, block, bsdf.wi);
			evaluateBsdf(wo, bsdf.wi, si, block, bsdf);
			out.set(block, bsdf);
		}

		virtual void evaluateWi(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, const DataBlock &block, Vec3f &wi) const
		{
			Onb uvw(si.n);
			wi = uvw.local(cosineSampleDirection(sample));
		}

		virtual void evaluateBsdf(const Vec3f &wo, const Vec3f &wi, const SurfaceInteraction &si, const DataBlock &block, BSDF &bsdf) const
		{
			Onb uvw(si.n);
			bsdf.pdf = dot(uvw.w, wi) * INV_PI;
			bsdf.scatteringPdf = scatteringPdf(si, wo, wi);
			bsdf.Li = f(wo, wi, block);
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

		ShadingOutput<BSDFSample> out;
	};
}