#pragma once

#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Material.h"

namespace atlas
{
    class MetalMaterial : public Material
    {
    public:
        struct Info
        {
            std::shared_ptr<Texture<Spectrum>> eta = nullptr;
            std::shared_ptr<Texture<Spectrum>> k = nullptr;
            std::shared_ptr<Texture<Float>> roughness = nullptr;
            std::shared_ptr<Texture<Float>> uRoughness = nullptr;
            std::shared_ptr<Texture<Float>> vRoughness = nullptr;
            std::shared_ptr<Texture<Float>> bumpMap = nullptr;
            bool remapRoughness = true;
        };

        ATLAS static std::shared_ptr<Material> create(const Info &info);

        ATLAS MetalMaterial(const Info &info);

        ATLAS MetalMaterial(const std::shared_ptr<Texture<Spectrum>> &eta,
            const std::shared_ptr<Texture<Spectrum>> &k,
            const std::shared_ptr<Texture<Float>> &rough,
            const std::shared_ptr<Texture<Float>> &urough,
            const std::shared_ptr<Texture<Float>> &vrough,
            const std::shared_ptr<Texture<Float>> &bump,
            bool remapRoughness);

        ATLAS void computeScatteringFunctions(SurfaceInteraction &si, TransportMode mode, bool allowMultipleLobes) const override;

    private:
        std::shared_ptr<Texture<Spectrum>> eta;
        std::shared_ptr<Texture<Spectrum>> k;
        std::shared_ptr<Texture<Float>> roughness;
        std::shared_ptr<Texture<Float>> uRoughness;
        std::shared_ptr<Texture<Float>> vRoughness;
        std::shared_ptr<Texture<Float>> bumpMap;
        bool remapRoughness;
    };

    typedef MetalMaterial::Info MetalMaterialInfo;
}