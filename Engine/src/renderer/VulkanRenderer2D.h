#pragma once

#include "Engine/src/renderer/RendererAPI.h"

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

class VulkanRenderer2D : public Renderer::RendererAPI {
private:
private:
    VulkanRenderer2D(std::unique_ptr<VulkanContext> &&context, SpriteRenderingPass &&spriteRenderingPass,
                     PostProcessingPass &&postProcessingPass, ImGuiRenderingPass &&imGuiRenderingPass);

public:
    ~VulkanRenderer2D() = default;

    VulkanRenderer2D(const VulkanRenderer2D &o) = delete;

    VulkanRenderer2D &operator=(const VulkanRenderer2D &o) = delete;

    VulkanRenderer2D(VulkanRenderer2D &&o) = delete;


    VulkanRenderer2D &operator=(VulkanRenderer2D &&o) = delete;

    static std::unique_ptr<VulkanRenderer2D> Create(Window &window);

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
    void draw(const glm::mat4 &modelMat, const RenderComponent &renderComponent);


private:
    void recreateSwapChain();

private:
    std::unique_ptr<VulkanContext> context;

    SpriteRenderingPass spriteRenderingPass;
    PostProcessingPass postProcessingPass;
    ImGuiRenderingPass imGuiRenderingPass;

    // TEMP
    RenderMesh quadMesh;

};

