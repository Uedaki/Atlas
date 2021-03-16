#include "atlas/core/Sampling.h"

using namespace atlas;

Vec3f atlas::uniformSampleSphere(const Point2f &u)
{
    Float z = 1 - 2 * u[0];
    Float r = std::sqrt(std::max((Float)0, (Float)1 - z * z));
    Float phi = 2 * PI * u[1];
    return Vec3f(r * std::cos(phi), r * std::sin(phi), z);
}

Vec3f atlas::uniformSampleHemisphere(const Point2f &u)
{
    Float z = u[0];
    Float r = std::sqrt(std::max((Float)0, (Float)1. - z * z));
    Float phi = 2 * PI * u[1];
    return Vec3f(r * std::cos(phi), r * std::sin(phi), z);
}

Float atlas::uniformConePdf(Float cosThetaMax)
{
    return 1 / (2 * PI * (1 - cosThetaMax));
}

void atlas::stratifiedSample1D(Float *samp, int nSamples, RNG &rng, bool jitter)
{
    Float invNSamples = (Float)1 / nSamples;
    for (int i = 0; i < nSamples; ++i)
    {
        Float delta = jitter ? rng.uniformFloat() : 0.5f;
        samp[i] = std::min((i + delta) * invNSamples, OneMinusEpsilon);
    }
}

void atlas::stratifiedSample2D(Point2f *samp, int nx, int ny, RNG &rng, bool jitter)
{
    Float dx = (Float)1 / nx, dy = (Float)1 / ny;
    for (int y = 0; y < ny; ++y)
        for (int x = 0; x < nx; ++x)
        {
            Float jx = jitter ? rng.uniformFloat() : 0.5f;
            Float jy = jitter ? rng.uniformFloat() : 0.5f;
            samp->x = std::min((x + jx) * dx, OneMinusEpsilon);
            samp->y = std::min((y + jy) * dy, OneMinusEpsilon);
            ++samp;
        }
}

void atlas::latinHypercube(Float *samples, int nSamples, int nDim, RNG &rng)
{
    Float invNSamples = (Float)1 / nSamples;
    for (int i = 0; i < nSamples; ++i)
        for (int j = 0; j < nDim; ++j)
        {
            Float sj = (i + (rng.uniformFloat())) * invNSamples;
            samples[nDim * i + j] = std::min(sj, OneMinusEpsilon);
        }

    for (int i = 0; i < nDim; ++i)
    {
        for (int j = 0; j < nSamples; ++j)
        {
            int other = j + rng.uniformUInt32(nSamples - j);
            std::swap(samples[nDim * j + i], samples[nDim * other + i]);
        }
    }
}

Point2f atlas::concentricSampleDisk(const Point2f &u)
{
    Point2f uOffset = 2.f * u - Vec2f(1, 1);

    if (uOffset.x == 0 && uOffset.y == 0)
        return Point2f(0, 0);

    Float theta, r;
    if (std::abs(uOffset.x) > std::abs(uOffset.y))
    {
        r = uOffset.x;
        theta = PI_OVER4 * (uOffset.y / uOffset.x);
    }
    else
    {
        r = uOffset.y;
        theta = PI_OVER2 - PI_OVER4 * (uOffset.x / uOffset.y);
    }
    return r * Point2f(std::cos(theta), std::sin(theta));
}

Float atlas::uniformHemispherePdf()
{
    return INV_2PI;
}