#include "atlas/materials/Metal.h"

using namespace atlas;

std::shared_ptr<Material> MetalMaterial::create(const Info &info)
{
    Info ci;
    ci.k = info.k ? info.k : atlas::createSpectrumConstant((Float)3.90463543, (Float)2.44763327, (Float)2.13765264);
    ci.eta = info.eta ? info.eta : atlas::createSpectrumConstant((Float)0.199990690, (Float)0.922084630, (Float)1.09987593);
    ci.roughness = info.roughness ? info.roughness : atlas::createFloatConstant((Float)0.01);
    ci.uRoughness = info.uRoughness;
    ci.vRoughness = info.vRoughness;
    ci.bumpMap = info.bumpMap;
    ci.remapRoughness = info.remapRoughness;
    return (std::make_shared<MetalMaterial>(ci));
}

MetalMaterial::MetalMaterial(const Info &info)
    : eta(info.eta)
    , k(info.k)
    , roughness(info.roughness)
    , uRoughness(info.uRoughness)
    , vRoughness(info.vRoughness)
    , bumpMap(info.bumpMap)
    , remapRoughness(info.remapRoughness)
{}

MetalMaterial::MetalMaterial(const std::shared_ptr<Texture<Spectrum>> &eta,
    const std::shared_ptr<Texture<Spectrum>> &k,
    const std::shared_ptr<Texture<Float>> &roughness,
    const std::shared_ptr<Texture<Float>> &uRoughness,
    const std::shared_ptr<Texture<Float>> &vRoughness,
    const std::shared_ptr<Texture<Float>> &bumpMap,
    bool remapRoughness)
    : eta(eta)
    , k(k)
    , roughness(roughness)
    , uRoughness(uRoughness)
    , vRoughness(vRoughness)
    , bumpMap(bumpMap)
    , remapRoughness(remapRoughness)
{}

void MetalMaterial::computeScatteringFunctions(SurfaceInteraction &si, TransportMode mode, bool allowMultipleLobes) const
{
    if (bumpMap)
        bump(bumpMap, si);
    si.bsdf = new BSDF(si);

    Float uRough = uRoughness ? uRoughness->evaluate(si) : roughness->evaluate(si);
    Float vRough = vRoughness ? vRoughness->evaluate(si) : roughness->evaluate(si);
    if (remapRoughness)
    {
        uRough = TrowbridgeReitzDistribution::RoughnessToAlpha(uRough);
        vRough = TrowbridgeReitzDistribution::RoughnessToAlpha(vRough);
    }
    Fresnel *frMf = new FresnelNoOp;
    //MicrofacetDistribution *distrib = new TrowbridgeReitzDistribution(uRough, vRough);
    //si.bsdf->add(new MicrofacetReflection(1., *distrib, *frMf));

    si.bsdf->add(new SpecularReflection(eta->evaluate(si), *frMf));
}