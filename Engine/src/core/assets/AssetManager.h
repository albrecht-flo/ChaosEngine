#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Engine/src/renderer/api/RenderMesh.h"
#include "Engine/src/renderer/api/Material.h"

class AssetManager {
public:
    enum class ResourceType {
        Scene, Entity, Mesh, Material, Texture, Script
    };

    struct MaterialInfo {
        bool hasTintColor;
    };

    struct MeshInfo {
    };
public:
    AssetManager() = default;

    ~AssetManager() = default;

    // ------------------------------------ Meshes ---------------------------------------------------------------------
    void registerMesh(const std::string &uri, const std::shared_ptr<Renderer::RenderMesh> &ref, MeshInfo meshInfo) {
        meshes.emplace(uri, std::make_pair(ref, meshInfo));
    }

    [[nodiscard]] Renderer::RenderMesh &getMesh(const std::string &uri) const { return *meshes.at(uri).first; }

    [[nodiscard]] MeshInfo getMeshInfo(const std::string &uri) const { return meshes.at(uri).second; }

    // ------------------------------------ Materials ------------------------------------------------------------------
    void registerMaterial(const std::string &uri, const Renderer::MaterialRef &ref, MaterialInfo materialInfo) {
        materials.emplace(uri, std::make_pair(ref, materialInfo));
    }

    [[nodiscard]] Renderer::MaterialRef getMaterial(const std::string &uri) const { return materials.at(uri).first; }

    [[nodiscard]] MaterialInfo getMaterialInfo(const std::string &uri) const { return materials.at(uri).second; }

private:
    std::unordered_map<std::string, std::pair<std::shared_ptr<Renderer::RenderMesh>, MeshInfo>> meshes{};
    std::unordered_map<std::string, std::pair<Renderer::MaterialRef, MaterialInfo>> materials{};
};



