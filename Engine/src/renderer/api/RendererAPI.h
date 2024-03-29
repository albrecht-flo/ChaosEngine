#pragma once

#include <string>
#include <utility>
#include <memory>
#include <optional>
#include <vector>
#include <cassert>

#include "Engine/src/core/Components.h"
#include "Engine/src/core/assets/Mesh.h"
#include "Engine/src/renderer/api/RenderPass.h"
#include "Engine/src/renderer/api/Material.h"
#include "Engine/src/renderer/api/Framebuffer.h"

namespace Renderer {

    struct DebugRenderData {
        std::vector<VertexPC> lines{};
    };

    enum class RendererType {
        RENDERER2D
    };

    // TODO Refactor using render pass approach (see Board 'March 29, 2022 1:05 PM')
    class RendererAPI {
    public:
        virtual ~RendererAPI() = default;

        // ------------------------------------ Lifecycle --------------------------------------------------------------

        /// Setup for all dynamic resources
        virtual void setup() = 0;

        /// Wait for GPU tasks to finish
        virtual void join() = 0;

        // ------------------------------------ Context commands -------------------------------------------------------

        /// Start recording commands with this renderer
        virtual void beginFrame() = 0;

        /// Finish this frame
        virtual void endFrame() = 0;

        /// Start recording commands to this renderers' scene
        virtual void beginScene(const glm::mat4 &viewMatrix, const CameraComponent &camera) = 0;

        /// Stop recording commands with this renderer
        virtual void endScene() = 0;

        /// Start recording commands to this renderers UI command buffer
        virtual void beginUI(const glm::mat4 &viewMat) = 0;

        /// Finalize the UI command buffer
        virtual void endUI() = 0;

        /// Start recording commands to this renderers TextOverlay command buffer
        virtual void beginTextOverlay(const glm::mat4 &viewMat) = 0;

        /// Finalize the TextOverlay command buffer
        virtual void endTextOverlay() = 0;

        /// Submit recorded commands to gpu
        virtual void flush() = 0;

        /// Resizes the scene viewport, after the next frame has been submited to the GPU
        virtual void requestViewportResize(const glm::vec2 &viewportSize) = 0;

        // ------------------------------------ Rendering commands -----------------------------------------------------

        /// Render an object with its material and model matrix
        virtual void draw(const glm::mat4 &viewMatrix, const RenderComponent &renderComponent) = 0;

        /// Render an indexed vertex buffer with its material
        virtual void drawText(const Buffer &vertexBuffer, const Buffer &indexBuffer,
                            uint32_t indexCount, uint32_t indexOffset,
                            const glm::mat4 &modelMat, const MaterialInstance &materialInstance) = 0;

        /// Render a mesh with a material and model matrix
        virtual void drawUI(const glm::mat4 &viewMatrix, const RenderMesh &mesh, const MaterialInstance &material) = 0;

        /// Render scene debug data
        virtual void drawSceneDebug(const glm::mat4 &viewMat, const CameraComponent &camera,
                                    const Renderer::DebugRenderData &debugRenderData) = 0;

        // ------------------------------------ Getters ----------------------------------------------------------------

        /// Gets the appropriate render pass for the requested shader stage
        [[nodiscard]] virtual const RenderPass &getRenderPassForShaderStage(ShaderPassStage stage) const = 0;

        [[nodiscard]] virtual const Renderer::Framebuffer &getFramebuffer() = 0;


    };
}
