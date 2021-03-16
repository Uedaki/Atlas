#pragma once

#include "atlas/Atlas.h"
#include "atlas/core/Ray.h"
#include "atlas/core/Sampler.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
    struct MediumInteraction;

    class PhaseFunction
    {
    public:
        virtual ~PhaseFunction() = default;
        virtual Float p(const Vec3f &wo, const Vec3f &wi) const = 0;
        virtual Float sampleP(const Vec3f &wo, Vec3f *wi, const Point2f &u) const = 0;
    };

    bool getMediumScatteringProperties(const std::string &name, Spectrum *sigma_a,
        Spectrum *sigma_s);

    inline Float phaseHG(Float cosTheta, Float g)
    {
        Float denom = 1 + g * g + 2 * g * cosTheta;
        return INV_4PI * (1 - g * g) / (denom * std::sqrt(denom));
    }

    class Medium
    {
    public:
        virtual ~Medium() {}
        virtual Spectrum tr(const Ray &ray, Sampler &sampler) const = 0;
        virtual Spectrum sample(const Ray &ray, Sampler &sampler, MediumInteraction &mi) const = 0;
    };

    struct MediumInterface
    {
        MediumInterface()
            : inside(nullptr), outside(nullptr)
        {}

        MediumInterface(const Medium *medium)
            : inside(medium), outside(medium)
        {}

        MediumInterface(const Medium *inside, const Medium *outside)
            : inside(inside), outside(outside)
        {}
        
        bool isMediumTransition() const
        {
            return inside != outside;
        }

        const Medium *inside;
        const Medium *outside;
    };
}