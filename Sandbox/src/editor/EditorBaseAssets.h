#pragma once

#include <memory>
#include "Engine/src/core/assets/AssetManager.h"
#include "Engine/src/renderer/api/RenderMesh.h"
#include "Engine/src/renderer/api/Texture.h"
#include "Engine/src/renderer/api/Material.h"
#include "EditorComponents.h"

namespace Editor {

    class EditorBaseAssets {
    public:
        explicit EditorBaseAssets(AssetManager &assetManager) : assetManager(assetManager) {}

        ~EditorBaseAssets() = default;

        EditorBaseAssets(const EditorBaseAssets &o) = delete;

        EditorBaseAssets &operator=(const EditorBaseAssets &o) = delete;

        EditorBaseAssets(EditorBaseAssets &&o) = delete;

        EditorBaseAssets &operator=(EditorBaseAssets &&o) = delete;

        // -------------------------------------------------------------------------------------------------------------

        void loadBaseMeshes();

        void loadBaseMaterials();

        void loadBaseTextures();

        // --------------------------------------- Materials -----------------------------------------------------------

        std::shared_ptr<Renderer::RenderMesh> getQuadMesh() const { return quadROB; }

        std::string getQuadMeshName() const { return "Quad"; }

        std::shared_ptr<Renderer::RenderMesh> getHexMesh() const { return hexROB; }

        std::string getHexMeshName() const { return "Hex"; }

        std::pair<std::shared_ptr<Renderer::RenderMesh>, std::string> getDefaultMesh() const {
            return std::make_pair(quadROB, getQuadMeshName());
        }

        // --------------------------------------- Materials -----------------------------------------------------------

        Renderer::MaterialRef getDebugMaterial() const { return debugMaterial; }

        Renderer::MaterialRef getTexturedMaterial() const { return texturedMaterial; }

        std::pair<Renderer::MaterialRef, std::string> getDefaultMaterial() const {
            return std::make_pair(texturedMaterial, "TexturedSprite");
        }

        // --------------------------------------- Textures ------------------------------------------------------------

        Renderer::Texture &getFallbackTexture() const { return *fallbackTexture; }

        std::string getFallbackTextureName() const { return "TestAtlas.jpg"; }

        std::pair<std::vector<const Renderer::Texture *>, std::vector<TextureMeta>>
        getDefaultTextureSet() const {
            auto info = std::vector<TextureMeta>(
                    {TextureMeta{"diffuse", getFallbackTextureName()}}
            );
            std::vector<const Renderer::Texture *> texs{fallbackTexture};
            return std::make_pair(texs, info);
        }

    private:
        AssetManager &assetManager;
        std::shared_ptr<Renderer::RenderMesh> quadROB;
        std::shared_ptr<Renderer::RenderMesh> hexROB;
        Renderer::MaterialRef debugMaterial = Renderer::MaterialRef(nullptr);
        Renderer::MaterialRef texturedMaterial = Renderer::MaterialRef(nullptr);
        Renderer::Texture *fallbackTexture = nullptr;
    public:
        uint32_t DebugMaterialID = 1;
        uint32_t TexturedMaterialID = 2;
    };

}
