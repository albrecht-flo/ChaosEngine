#pragma once

#include <string>
#include <unordered_map>

#include "Engine/src/renderer/api/Material.h"

class AssetManager {
public:
    enum class ResourceType {
        Scene, Entity, Mesh, Material, Texture, Script
    };

    struct MaterialInfo {
        bool hasTintColor;
    };

public:
    AssetManager() = default;

    ~AssetManager() = default;

    void registerMaterial(const std::string &uri, const Renderer::MaterialRef& ref, MaterialInfo materialInfo) {
        materials.emplace(uri, std::make_pair(ref, materialInfo));
    }

    [[nodiscard]] Renderer::MaterialRef getMaterial(const std::string &uri) const { return materials.at(uri).first; }

    [[nodiscard]] MaterialInfo getMaterialInfo(const std::string &uri) const { return materials.at(uri).second; }

private:
    std::unordered_map<std::string, std::pair<Renderer::MaterialRef, MaterialInfo>> materials{};
};



