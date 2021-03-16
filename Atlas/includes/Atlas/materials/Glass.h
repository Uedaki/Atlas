#pragma once

#include "atlas/AtlasLibHeader.h"
#include "atlas/core/Material.h"

namespace atlas
{
    class GlassMaterial : public Material
    {
    public:
        struct Info
        {
            std::shared_ptr<Texture<Spectrum>> kr = nullptr; // default 1, 1, 1
            std::shared_ptr<Texture<Spectrum>> kt = nullptr; // default 1, 1, 1
            std::shared_ptr<Texture<Float>> uRoughness = nullptr; // default 0
            std::shared_ptr<Texture<Float>> vRoughness = nullptr; // default 0
            std::shared_ptr<Texture<Float>> index = nullptr; // default 1.5
            std::shared_ptr<Texture<Float>> bumpMap = nullptr; // default null
            bool remapRoughness = true; // default true
        };

        ATLAS static std::shared_ptr<Material> create(const Info &info = Info());

        GlassMaterial(const Info &info)
            : kr(info.kr), kt(info.kt)
            , uRoughness(info.uRoughness)
            , vRoughness(info.vRoughness)
            , index(info.index)
            , bumpMap(info.bumpMap)
            , remapRoughness(info.remapRoughness)
        {}

        GlassMaterial(const std::shared_ptr<Texture<Spectrum>> &kr,
            const std::shared_ptr<Texture<Spectrum>> &kt,
            const std::shared_ptr<Texture<Float>> &uRoughness,
            const std::shared_ptr<Texture<Float>> &vRoughness,
            const std::shared_ptr<Texture<Float>> &index,
            const std::shared_ptr<Texture<Float>> &bumpMap,
            bool remapRoughness)
            : kr(kr), kt(kt)
            , uRoughness(uRoughness)
            , vRoughness(vRoughness)
            , index(index)
            , bumpMap(bumpMap)
            , remapRoughness(remapRoughness)
        {}

        ATLAS void computeScatteringFunctions(SurfaceInteraction &si, TransportMode mode, bool allowMultipleLobes) const override;

    private:
        std::shared_ptr<Texture<Spectrum>> kr;
        std::shared_ptr<Texture<Spectrum>> kt;
        std::shared_ptr<Texture<Float>> uRoughness;
        std::shared_ptr<Texture<Float>> vRoughness;
        std::shared_ptr<Texture<Float>> index;
        std::shared_ptr<Texture<Float>> bumpMap;
        bool remapRoughness;
    };

    typedef GlassMaterial::Info GlassMaterialInfo;
}