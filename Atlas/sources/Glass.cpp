#include "atlas/materials/Glass.h"

using namespace atlas;

std::shared_ptr<Material> GlassMaterial::create(const Info &info)
{
    Info ci;
    ci.kr = info.kr ? info.kr : atlas::createSpectrumConstant(1.f, 1.f, 1.f);
    ci.kt = info.kt ? info.kt : atlas::createSpectrumConstant(1.f, 1.f, 1.f);
    ci.uRoughness = info.uRoughness ? info.uRoughness : atlas::createFloatConstant(0.f);
    ci.vRoughness = info.vRoughness ? info.vRoughness : atlas::createFloatConstant(0.f);
    ci.index = info.index ? info.index : atlas::createFloatConstant(1.5f);
    ci.bumpMap = info.bumpMap;
    ci.remapRoughness = info.remapRoughness;
    return (std::make_shared<GlassMaterial>(ci));
}

void GlassMaterial::computeScatteringFunctions(SurfaceInteraction &si, TransportMode mode, bool allowMultipleLobes) const
{
    if (bumpMap)
        bump(bumpMap, si);
    Float eta = index->evaluate(si);
    Float urough = uRoughness->evaluate(si);
    Float vrough = vRoughness->evaluate(si);
    Spectrum R = kr->evaluate(si).getClampedSpectrum(0, 1);
    Spectrum T = kt->evaluate(si).getClampedSpectrum(0, 1);

    si.bsdf = new BSDF(si, eta);

    if (R.isBlack() && T.isBlack())
        return;

    bool isSpecular = urough == 0 && vrough == 0;

    {
        if (remapRoughness)
        {
            urough = TrowbridgeReitzDistribution::RoughnessToAlpha(urough);
            vrough = TrowbridgeReitzDistribution::RoughnessToAlpha(vrough);
        }
        MicrofacetDistribution *distrib =
            isSpecular ? nullptr
            : new TrowbridgeReitzDistribution(urough, vrough);

            si.bsdf->add(new SpecularTransmission(T, 1.f, eta, mode));

    }
}