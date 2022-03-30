#pragma once

#include "renderer/api/RendererAPI.h"
#include "TestContext.h"
#include "TestRenderPass.h"

namespace Renderer::TestRenderer {

    class TestRenderer : public Renderer::RendererAPI {
    public:
        explicit TestRenderer(TestContext &context, TestRenderPass &&testPass) : context(context),
                                                                                 testPass(std::move(testPass)) {}

        ~TestRenderer() override = default;

        TestRenderer(const TestRenderer &o) = delete;

        TestRenderer &operator=(const TestRenderer &o) = delete;

        TestRenderer(TestRenderer &&o) = delete;

        TestRenderer &operator=(TestRenderer &&o) = delete;

        static std::unique_ptr<TestRenderer> Create(Renderer::GraphicsContext &graphicsContext);

        // Lifecycle
        /// Setup for all dynamic resources
        void setup() override;

        /// Wait for GPU tasks to finish
        void join() override;

        // Context commands
        /// Start recording commands with this renderer
        void beginFrame() override;

        /// Finish this frame
        void endFrame() override;

        /// Start recording commands with this renderer
        void beginScene(const glm::mat4 &viewMat, const CameraComponent &camera) override;

        /// Stop recording commands with this renderer
        void endScene() override;

        /// Start recording commands to this renderers UI command buffer
        virtual void beginUI(const glm::mat4 &viewMat) override;

        /// Finalize the UI command buffer
        virtual void endUI() override;

        /// Submit recorded commands to gpu
        void flush() override;

        /// Resizes the scene viewport
        void requestViewportResize(const glm::vec2 &viewportSize) override;

        // Rendering commands
        /// Render an object with its material and model matrix
        void draw(const glm::mat4 &modelMat, const RenderComponent &renderComponent) override;

        /// Render an indexed vertex buffer with its material
        virtual void drawUI(const Buffer &vertexBuffer, const Buffer &indexBuffer,
                            uint32_t indexCount, uint32_t indexOffset,
                            const glm::mat4 &modelMat, const MaterialInstance &materialInstance) override;

        /// Gets the appropriate render pass for the requested shader stage
        [[nodiscard]] const Renderer::RenderPass &
        getRenderPassForShaderStage(Renderer::ShaderPassStage stage) const override;

        const Renderer::Framebuffer &getFramebuffer() override;

    private:
        TestContext &context;
        TestRenderPass testPass;
    };

}

