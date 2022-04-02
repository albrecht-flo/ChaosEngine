#pragma once

#include "Engine/src/renderer/api/RendererAPI.h"

#include "Engine/src/core/Components.h"
#include "Engine/src/renderer/passes/SpriteRenderingPass.h"
#include "Engine/src/renderer/passes/UIRenderingPass.h"
#include "Engine/src/renderer/passes/ImGuiRenderingPass.h"
#include "Engine/src/renderer/passes/PostProcessingPass.h"
#include "Engine/src/renderer/api/RenderMesh.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanFrame.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanVertexInput.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "Engine/src/renderer/vulkan/image/VulkanImage.h"
#include "Engine/src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "Engine/src/renderer/api/Material.h"

class VulkanRenderer2D : public Renderer::RendererAPI {
private:
private:
    VulkanRenderer2D(VulkanContext &context,
                     SpriteRenderingPass &&spriteRenderingPass,
                     UIRenderingPass &&uiRenderPass,
                     PostProcessingPass &&postProcessingPass,
                     ImGuiRenderingPass &&imGuiRenderingPass,
                     bool renderingSceneToSwapchain);

public:
    ~VulkanRenderer2D() = default;

    VulkanRenderer2D(const VulkanRenderer2D &o) = delete;

    VulkanRenderer2D &operator=(const VulkanRenderer2D &o) = delete;

    VulkanRenderer2D(VulkanRenderer2D &&o) = delete;

    VulkanRenderer2D &operator=(VulkanRenderer2D &&o) = delete;

    static std::unique_ptr<VulkanRenderer2D> Create(Renderer::GraphicsContext &graphicsContext);

    // Lifecycle

    /// Setup for all dynamic resources
    void setup() override;

    /// Wait for GPU tasks to finish
    void join() override;

    // Context commands
    /// Start recording commands with this renderer
    void beginFrame() override;

    /// Start recording commands with this renderer
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
    void drawUI(const Renderer::Buffer &vertexBuffer, const Renderer::Buffer &indexBuffer,
                uint32_t indexCount, uint32_t indexOffset,
                const glm::mat4 &modelMat, const Renderer::MaterialInstance &materialInstance) override;

    /// Render a mesh with a material and model matrix
    void drawUI(const glm::mat4 &viewMatrix, const Renderer::RenderMesh &mesh,
                const Renderer::MaterialInstance &material) override;

    /// Gets the appropriate render pass for the requested shader stage
    [[nodiscard]] const Renderer::RenderPass &
    getRenderPassForShaderStage(Renderer::ShaderPassStage stage) const override;

    const Renderer::Framebuffer &getFramebuffer() override {
        return postProcessingPass.getColorAttachment();
    }

private:
    void recreateSwapChain();

private:
    VulkanContext &context;

    SpriteRenderingPass spriteRenderingPass;
    UIRenderingPass uiRenderingPass;
    PostProcessingPass postProcessingPass;
    ImGuiRenderingPass imGuiRenderingPass;

    bool renderingSceneToSwapchain;
    glm::uvec2 sceneResize{0, 0};

};

