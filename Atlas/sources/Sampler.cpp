#include "atlas/core/Sampler.h"

#include "atlas/core/Camera.h"
#include "atlas/core/Random.h"
#include "atlas/core/Sampling.h"

using namespace atlas;

Sampler::Sampler(int64_t samplesPerPixel)
    : samplesPerPixel(samplesPerPixel)
{}

CameraSample Sampler::getCameraSample(const Point2i &pRaster)
{
    CameraSample cs;
    cs.pFilm = (Point2f)pRaster + get2D();
    cs.time = get1D();
    cs.pLens = get2D();
    return cs;
}

void Sampler::startPixel(const Point2i &p)
{
    currentPixel = p;
    currentPixelSampleIndex = 0;
    array1DOffset = array2DOffset = 0;
}

bool Sampler::startNextSample()
{
    array1DOffset = array2DOffset = 0;
    return ++currentPixelSampleIndex < samplesPerPixel;
}

bool Sampler::setSampleNumber(int64_t sampleNum)
{
    array1DOffset = array2DOffset = 0;
    currentPixelSampleIndex = sampleNum;
    return currentPixelSampleIndex < samplesPerPixel;
}

void Sampler::request1DArray(int n)
{
    samples1DArraySizes.push_back(n);
    sampleArray1D.push_back(std::vector<Float>(n * samplesPerPixel));
}

void Sampler::request2DArray(int n)
{
    samples2DArraySizes.push_back(n);
    sampleArray2D.push_back(std::vector<Point2f>(n * samplesPerPixel));
}

const Float *Sampler::get1DArray(int n)
{
    if (array1DOffset == sampleArray1D.size())
        return nullptr;
    return &sampleArray1D[array1DOffset++][currentPixelSampleIndex * n];
}

const Point2f *Sampler::get2DArray(int n)
{
    if (array2DOffset == sampleArray2D.size())
        return nullptr;
    return &sampleArray2D[array2DOffset++][currentPixelSampleIndex * n];
}

PixelSampler::PixelSampler(int64_t samplesPerPixel, int nSampledDimensions)
    : Sampler(samplesPerPixel)
{
    for (int i = 0; i < nSampledDimensions; ++i) {
        samples1D.push_back(std::vector<Float>(samplesPerPixel));
        samples2D.push_back(std::vector<Point2f>(samplesPerPixel));
    }
}

bool PixelSampler::startNextSample()
{
    current1DDimension = current2DDimension = 0;
    return Sampler::startNextSample();
}

bool PixelSampler::setSampleNumber(int64_t sampleNum)
{
    current1DDimension = current2DDimension = 0;
    return Sampler::setSampleNumber(sampleNum);
}

Float PixelSampler::get1D()
{
    if (current1DDimension < samples1D.size())
        return samples1D[current1DDimension++][currentPixelSampleIndex];
    else
        return rng.uniformFloat();
}

Point2f PixelSampler::get2D()
{
    if (current2DDimension < samples2D.size())
        return samples2D[current2DDimension++][currentPixelSampleIndex];
    else
        return Point2f(rng.uniformFloat(), rng.uniformFloat());
}

void GlobalSampler::startPixel(const Point2i &p)
{
    Sampler::startPixel(p);
    dimension = 0;
    intervalSampleIndex = getIndexForSample(0);
    arrayEndDim = arrayStartDim + sampleArray1D.size() + 2 * sampleArray2D.size();

    for (size_t i = 0; i < samples1DArraySizes.size(); ++i)
    {
        int nSamples = samples1DArraySizes[i] * samplesPerPixel;
        for (int j = 0; j < nSamples; ++j)
        {
            int64_t index = getIndexForSample(j);
            sampleArray1D[i][j] = sampleDimension(index, arrayStartDim + i);
        }
    }

    int dim = arrayStartDim + samples1DArraySizes.size();
    for (size_t i = 0; i < samples2DArraySizes.size(); ++i)
    {
        int nSamples = samples2DArraySizes[i] * samplesPerPixel;
        for (int j = 0; j < nSamples; ++j)
        {
            int64_t idx = getIndexForSample(j);
            sampleArray2D[i][j].x = sampleDimension(idx, dim);
            sampleArray2D[i][j].y = sampleDimension(idx, dim + 1);
        }
        dim += 2;
    }
}

bool GlobalSampler::startNextSample()
{
    dimension = 0;
    intervalSampleIndex = getIndexForSample(currentPixelSampleIndex + 1);
    return Sampler::startNextSample();
}

bool GlobalSampler::setSampleNumber(int64_t sampleNum)
{
    dimension = 0;
    intervalSampleIndex = getIndexForSample(sampleNum);
    return Sampler::setSampleNumber(sampleNum);
}

Float GlobalSampler::get1D()
{
    if (dimension >= arrayStartDim && dimension < arrayEndDim)
        dimension = arrayEndDim;
    return sampleDimension(intervalSampleIndex, dimension++);
}

Point2f GlobalSampler::get2D()
{
    if (dimension + 1 >= arrayStartDim && dimension < arrayEndDim)
        dimension = arrayEndDim;
    Point2f p(sampleDimension(intervalSampleIndex, dimension),
        sampleDimension(intervalSampleIndex, dimension + 1));
    dimension += 2;
    return p;
}

void StratifiedSampler::startPixel(const Point2i &p)
{
    for (size_t i = 0; i < samples1D.size(); ++i)
    {
        stratifiedSample1D(&samples1D[i][0], xPixelSamples * yPixelSamples, rng, jitterSamples);
        shuffle(&samples1D[i][0], xPixelSamples * yPixelSamples, 1, rng);
    }
    for (size_t i = 0; i < samples2D.size(); ++i)
    {
        stratifiedSample2D(&samples2D[i][0], xPixelSamples, yPixelSamples, rng,
            jitterSamples);
        shuffle(&samples2D[i][0], xPixelSamples * yPixelSamples, 1, rng);
    }

    for (size_t i = 0; i < samples1DArraySizes.size(); ++i)
        for (int64_t j = 0; j < samplesPerPixel; ++j)
        {
            int count = samples1DArraySizes[i];
            stratifiedSample1D(&sampleArray1D[i][j * count], count, rng,
                jitterSamples);
            shuffle(&sampleArray1D[i][j * count], count, 1, rng);
        }
    for (size_t i = 0; i < samples2DArraySizes.size(); ++i)
        for (int64_t j = 0; j < samplesPerPixel; ++j)
        {
            int count = samples2DArraySizes[i];
            latinHypercube(&sampleArray2D[i][j * count].x, count, 2, rng);
        }
    PixelSampler::startPixel(p);
}

std::unique_ptr<Sampler> StratifiedSampler::clone(int seed)
{
    StratifiedSampler *ss = new StratifiedSampler(*this);
    ss->rng.setSequence(seed);
    return std::unique_ptr<Sampler>(ss);
}