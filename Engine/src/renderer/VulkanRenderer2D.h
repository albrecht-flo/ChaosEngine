#pragma once

#include "Engine/src/renderer/api/RendererAPI.h"

#include "Engine/src/core/Components.h"
#include "Engine/src/renderer/passes/SpriteRenderingPass.h"
#include "Engine/src/renderer/passes/ImGuiRenderingPass.h"
#include "Engine/src/renderer/passes/PostProcessingPass.h"
#include "Engine/src/renderer/data/RenderObject.h"
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
    VulkanRenderer2D(VulkanContext &context, SpriteRenderingPass &&spriteRenderingPass,
                     PostProcessingPass &&postProcessingPass, ImGuiRenderingPass &&imGuiRenderingPass);

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
    void beginScene(const glm::mat4 &viewMat, const CameraComponent &camera) override;

    /// Stop recording commands with this renderer
    void endScene() override;

    /// Submit recorded commands to gpu
    void flush() override;

    // Rendering commands
    /// Render an object with its material and model matrix
    void draw(const glm::mat4 &modelMat, const RenderComponent &renderComponent) override;

    /// Gets the apropriate render pass for the requested shader stage
    const Renderer::RenderPass &getRenderPassForShaderStage(Renderer::ShaderPassStage stage) const override;

    Renderer::Texture & getRendererTexture() override { // TODO: IN Progress
        const auto& color = postProcessingPass.getColorAttachment();
        if(color.getImage() != renderImage) {
            auto x = VulkanImageView::Create(context.getDevice(), color.getImage(), VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_IMAGE_ASPECT_COLOR_BIT);
            renderImage = color.getImage();
            renderTexture = std::make_unique<VulkanTexture>(context.getDevice(), color.getImage(), nullptr,
                                                   color.getImageLayout(), std::move(x),
                                                   VulkanSampler::create(context.getDevice()),
                                                   color.getWidth(), color.getHeight());
        }
        return *renderTexture;
    }

private:
    void recreateSwapChain();

private:
    VulkanContext &context;

    SpriteRenderingPass spriteRenderingPass;
    PostProcessingPass postProcessingPass;
    ImGuiRenderingPass imGuiRenderingPass;

    // TEMP
    RenderMesh quadMesh;

    VkImage renderImage = VK_NULL_HANDLE;
    std::unique_ptr<VulkanTexture> renderTexture = nullptr;
};

