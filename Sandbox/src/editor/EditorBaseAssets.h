#pragma once

#include <memory>
#include "Engine/src/renderer/api/RenderMesh.h"
#include "Engine/src/renderer/api/Texture.h"
#include "Engine/src/renderer/api/Material.h"

namespace Editor {

    class EditorBaseAssets {
    public:
        EditorBaseAssets() = default;

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

        std::shared_ptr<Renderer::RenderMesh> getHexMesh() const { return hexROB; }


        // --------------------------------------- Materials -----------------------------------------------------------

        Renderer::MaterialRef getDebugMaterial() const { return debugMaterial; }

        Renderer::MaterialRef getTexturedMaterial() const { return texturedMaterial; }

        // --------------------------------------- Textures ------------------------------------------------------------

        Renderer::Texture &getFallbackTexture() const { return *fallbackTexture; }


    private:
        std::shared_ptr<Renderer::RenderMesh> quadROB;
        std::shared_ptr<Renderer::RenderMesh> hexROB;
        Renderer::MaterialRef debugMaterial = Renderer::MaterialRef(nullptr);
        Renderer::MaterialRef texturedMaterial = Renderer::MaterialRef(nullptr);
        std::unique_ptr<Renderer::Texture> fallbackTexture = nullptr;
    };

}
