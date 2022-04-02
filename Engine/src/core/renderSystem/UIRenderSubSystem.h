#pragma once

#include "Engine/src/core/Ecs.h"
#include "Engine/src/core/assets/Mesh.h"
#include "Engine/src/renderer/api/RendererAPI.h"
#include "Engine/src/renderer/api/Buffer.h"

namespace ChaosEngine {

    class UIRenderSubSystem {
    public:
        ~UIRenderSubSystem() = default;

        void init(uint32_t glyphCapacity = 2048);

        void render(ECS &ecs, Renderer::RendererAPI &renderer);

    private:
        uint32_t renderTextToBuffers(uint32_t bufferOffsetInGlyphs, VertexPCU *vBufferRef, uint32_t *iBufferRef,
                                     const UITextComponent &text, glm::vec3 linePos) const;

    private:
        uint32_t currentBufferedFrame = 0;
        uint32_t glyphCapacity = 0;
        std::vector<std::unique_ptr<Renderer::Buffer>> textVertexBuffers{};
        std::vector<std::unique_ptr<Renderer::Buffer>> textIndexBuffers{};
        Renderer::MaterialRef uiTextMaterial = Renderer::MaterialRef(nullptr);
        std::unordered_map<const Font *, std::shared_ptr<Renderer::MaterialInstance>> fontMaterialInstances{};
    };

}


