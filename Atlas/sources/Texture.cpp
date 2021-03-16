#include "atlas/core/Texture.h"

using namespace atlas;

std::shared_ptr<ConstantTexture<Float>> atlas::createFloatConstant(Float s)
{
    return (std::make_shared<ConstantTexture<Float>>(s));
}

std::shared_ptr<ConstantTexture<Spectrum>> atlas::createSpectrumConstant(const Spectrum &s)
{
    return (std::make_shared<ConstantTexture<Spectrum>>(s));
}

std::shared_ptr<ConstantTexture<Spectrum>> atlas::createSpectrumConstant(Float r, Float g, Float b)
{
    return (std::make_shared<ConstantTexture<Spectrum>>(Spectrum(r, g, b)));
}