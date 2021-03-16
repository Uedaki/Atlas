#pragma once

#include <vector>

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Points.h"
#include "atlas/core/Random.h"

namespace atlas
{
	struct CameraSample;

	class Sampler
	{
	public:
		ATLAS Sampler(int64_t samplesPerPixel);
		virtual ~Sampler() = default;

		virtual void startPixel(const atlas::Point2i &p) = 0;
		virtual Float get1D() = 0;
		virtual atlas::Point2f get2D() = 0;
		virtual bool startNextSample() = 0;
		virtual bool setSampleNumber(int64_t sampleNum) = 0;
		
		ATLAS CameraSample getCameraSample(const atlas::Point2i &p);

		ATLAS void request1DArray(int n);
		ATLAS void request2DArray(int n);

		virtual int roundCount(int n) const
		{
			return n;
		}

		ATLAS const Float *get1DArray(int n);
		ATLAS const Point2f *get2DArray(int n);

		virtual std::unique_ptr<Sampler> clone(int seed) = 0;

		const int64_t samplesPerPixel;
	protected:
		Point2i currentPixel;
		int64_t currentPixelSampleIndex;
		std::vector<int> samples1DArraySizes;
		std::vector<int> samples2DArraySizes;
		std::vector<std::vector<Float>> sampleArray1D;
		std::vector<std::vector<Point2f>> sampleArray2D;

	private:
		size_t array1DOffset, array2DOffset;
	};

	class PixelSampler : public Sampler
	{
	public:
		ATLAS PixelSampler(int64_t samplesPerPixel, int nSampledDimensions);
		ATLAS bool startNextSample() override;
		ATLAS bool setSampleNumber(int64_t) override;
		ATLAS Float get1D() override;
		ATLAS Point2f get2D() override;

	protected:
		std::vector<std::vector<Float>> samples1D;
		std::vector<std::vector<Point2f>> samples2D;
		int current1DDimension = 0;
		int current2DDimension = 0;
		RNG rng;
	};

	class GlobalSampler : public Sampler
	{
	public:
		ATLAS bool startNextSample() override;
		ATLAS void startPixel(const Point2i &) override;
		ATLAS bool setSampleNumber(int64_t sampleNum) override;
		ATLAS Float get1D() override;
		ATLAS Point2f get2D() override;
		
		GlobalSampler(int64_t samplesPerPixel)
			: Sampler(samplesPerPixel)
		{}

		virtual int64_t getIndexForSample(int64_t sampleNum) const = 0;
		virtual Float sampleDimension(int64_t index, int dimension) const = 0;

	private:
		int dimension;
		int64_t intervalSampleIndex;
		static const int arrayStartDim = 5;
		int arrayEndDim;
	};

	class StratifiedSampler : public PixelSampler {
	public:
		struct Info
		{
			bool jitter = true;
			int xsamp = 4;
			int ysamp = 4;
			int sd = 4;
		};

		static Sampler *create(const Info &info)
		{
			return (new StratifiedSampler(info));
		}

		StratifiedSampler(const Info &info)
			: PixelSampler(info.xsamp * info.ysamp, info.sd)
			, xPixelSamples(info.xsamp)
			, yPixelSamples(info.ysamp)
			, jitterSamples(info.jitter)
		{}

		StratifiedSampler(int xPixelSamples, int yPixelSamples, bool jitterSamples, int nSampledDimensions)
			: PixelSampler(xPixelSamples * yPixelSamples, nSampledDimensions)
			, xPixelSamples(xPixelSamples)
			, yPixelSamples(yPixelSamples)
			, jitterSamples(jitterSamples)
		{}

		ATLAS void startPixel(const Point2i &);
		
		ATLAS std::unique_ptr<Sampler> clone(int seed);

	private:
		const int xPixelSamples;
		const int yPixelSamples;
		const bool jitterSamples;
	};

	typedef StratifiedSampler::Info StratifiedSamplerInfo;
}