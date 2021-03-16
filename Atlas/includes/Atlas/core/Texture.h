#pragma once

#include <memory>

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Interaction.h"
#include "atlas/core/RgbSpectrum.h"

namespace atlas
{
    template <typename T>
    class Texture {
    public:
        virtual T evaluate(const SurfaceInteraction &) const = 0;
        virtual ~Texture() {}
    };

    template <typename T>
    class ConstantTexture : public Texture<T>
    {
    public:
        ConstantTexture(const T &value)
            : value(value)
        {}
        
        T evaluate(const SurfaceInteraction &) const
        {
            return value;
        }

    private:
        T value;
    };

    ATLAS std::shared_ptr<ConstantTexture<Float>> createFloatConstant(Float s);
    ATLAS std::shared_ptr<ConstantTexture<Spectrum>> createSpectrumConstant(const Spectrum &s);
    ATLAS std::shared_ptr<ConstantTexture<Spectrum>> createSpectrumConstant(Float r, Float g, Float b);
}