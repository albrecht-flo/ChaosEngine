#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

#include "Engine/src/renderer/api/RenderMesh.h"
#include "Engine/src/renderer/api/Material.h"
#include "Engine/src/renderer/api/Texture.h"
#include "Engine/src/core/Entity.h"
#include "Engine/src/core/Components.h"

namespace ChaosEngine {

    class AssetManager {
    public:
        using NativeScriptCreator = std::function<std::unique_ptr<NativeScript>(Entity)>;
        enum class ResourceType {
            Scene, Entity, Mesh, Material, Texture, Script
        };

        struct MaterialInfo {
            bool hasTintColor;
        };

        struct MeshInfo {
        };

        struct TextureInfo {
        };

        struct ScriptInfo {
        };
    public:
        AssetManager() = default;

        ~AssetManager() = default;

        // ------------------------------------ Meshes -----------------------------------------------------------------
        void registerMesh(const std::string &uri, const std::shared_ptr<Renderer::RenderMesh> &ref, MeshInfo meshInfo) {
            meshes.emplace(uri, std::make_pair(ref, meshInfo));
        }

        [[nodiscard]] std::shared_ptr<Renderer::RenderMesh> getMesh(const std::string &uri) const {
            return meshes.at(uri).first;
        }

        [[nodiscard]] MeshInfo getMeshInfo(const std::string &uri) const { return meshes.at(uri).second; }

        [[nodiscard]] const std::unordered_map<std::string, std::pair<std::shared_ptr<Renderer::RenderMesh>, MeshInfo>> &
        getAllMeshes() const { return meshes; }

        // ------------------------------------ Materials --------------------------------------------------------------
        void registerMaterial(const std::string &uri, const Renderer::MaterialRef &ref, MaterialInfo materialInfo) {
            materials.emplace(uri, std::make_pair(ref, materialInfo));
        }

        [[nodiscard]] Renderer::MaterialRef getMaterial(const std::string &uri) const {
            return materials.at(uri).first;
        }

        [[nodiscard]] MaterialInfo getMaterialInfo(const std::string &uri) const { return materials.at(uri).second; }

        [[nodiscard]] const std::unordered_map<std::string, std::pair<Renderer::MaterialRef, MaterialInfo>> &
        getAllMaterials() const { return materials; }

        // ------------------------------------ Textures ---------------------------------------------------------------
        Renderer::Texture *
        registerTexture(const std::string &uri, std::unique_ptr<Renderer::Texture> &&texture, TextureInfo texInfo) {
            auto tex = textures.emplace(uri, std::make_pair(std::move(texture), texInfo));
            return tex.first->second.first.get();
        }

        [[nodiscard]] Renderer::Texture &getTexture(const std::string &uri) const { return *textures.at(uri).first; }

        [[nodiscard]] TextureInfo getTextureInfo(const std::string &uri) const { return textures.at(uri).second; }

        [[nodiscard]] const std::unordered_map<std::string, std::pair<std::unique_ptr<Renderer::Texture>, TextureInfo>> &
        getAllTextures() const { return textures; }

        // ------------------------------------ Native Scripts ---------------------------------------------------------
        void
        registerNativeScript(const std::string &uri, const NativeScriptCreator &nativeScriptCreator,
                             ScriptInfo texInfo) {
            scripts.emplace(uri, std::make_pair(nativeScriptCreator, texInfo));
        }

        [[nodiscard]] std::unique_ptr<NativeScript> getScript(const std::string &uri, Entity entity) const {
            return (scripts.at(uri).first)(entity);
        }

        [[nodiscard]] ScriptInfo getScriptInfo(const std::string &uri) const { return scripts.at(uri).second; }

        [[nodiscard]] const std::unordered_map<std::string, std::pair<NativeScriptCreator, ScriptInfo>> &
        getAllScripts() const { return scripts; }

    private:
        std::unordered_map<std::string, std::pair<std::shared_ptr<Renderer::RenderMesh>, MeshInfo>> meshes{};
        std::unordered_map<std::string, std::pair<Renderer::MaterialRef, MaterialInfo>> materials{};
        std::unordered_map<std::string, std::pair<std::unique_ptr<Renderer::Texture>, TextureInfo>> textures{};
        std::unordered_map<std::string, std::pair<NativeScriptCreator, ScriptInfo>> scripts{};
    };

}
