#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Points.h"
#include "atlas/core/Random.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
    inline bool sameHemisphere(const Vec3f &w, const Vec3f &wp)
    {
        return w.z * wp.z > 0;
    }

	ATLAS Vec3f uniformSampleSphere(const Point2f &u);

    ATLAS Vec3f uniformSampleHemisphere(const Point2f &u);

	ATLAS Float uniformConePdf(Float cosThetaMax);

	ATLAS void stratifiedSample1D(Float *samp, int nSamples, RNG &rng, bool jitter);
    ATLAS void stratifiedSample2D(Point2f *samp, int nx, int ny, RNG &rng, bool jitter);

    ATLAS void latinHypercube(Float *samples, int nSamples, int nDim, RNG &rng);

    ATLAS Float uniformHemispherePdf();

    ATLAS Point2f concentricSampleDisk(const Point2f &u);

    inline Vec3f cosineSampleDirection(const Point2f &u)
    {
        const Float r1 = u.x;
        const Float r2 = u.y;
        const Float z = std::sqrt(1 - r2);

        const Float phi = 2 * PI * r1;
        const Float x = cos(phi) * std::sqrt(r2);
        const Float y = sin(phi) * std::sqrt(r2);

        return (Vec3f(x, y, z));
    }

    inline Vec3f cosineSampleHemisphere(const Point2f &u)
    {
        Point2f d = concentricSampleDisk(u);
        Float z = std::sqrt(std::max((Float)0, 1 - d.x * d.x - d.y * d.y));
        return (Vec3f(d.x, d.y, z));
    }

    inline Float cosineHemispherePdf(Float cosTheta)
    {
        return cosTheta * INV_PI;
    }

    inline Point2f uniformSampleTriangle(const Point2f &u)
    {
        Float su0 = std::sqrt(u[0]);
        return Point2f(1 - su0, u[1] * su0);
    }

    template <typename T>
    void shuffle(T *samp, int count, int nDimensions, RNG &rng)
    {
        for (int i = 0; i < count; ++i)
        {
            int other = i + rng.uniformUInt32(count - i);
            for (int j = 0; j < nDimensions; ++j)
                std::swap(samp[nDimensions * i + j], samp[nDimensions * other + j]);
        }
    }
}