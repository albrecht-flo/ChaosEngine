#pragma once

#include "Engine/src/core/Components.h"
#include "Engine/src/renderer/vulkan/api/VulkanRenderMesh.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "Engine/src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanVertexInput.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPool.h"
#include "Engine/src/renderer/vulkan/api/VulkanMaterial.h"

#include <string>

// ------------------------------------ Pipeline Description -----------------------------------------------------------
class VulkanMaterialInstance;

class UIRenderingPass {
private:
    /// Canvas uniform object. Alignas 16 to ensure proper alignment with the GPU storage
    struct CanvasUbo {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };
private:
    explicit UIRenderingPass(const VulkanContext &context) : context(context) {}

    void init(uint32_t width, uint32_t height);

public:
    ~UIRenderingPass() = default;

    UIRenderingPass(const UIRenderingPass &o) = delete;

    UIRenderingPass &operator=(const UIRenderingPass &o) = delete;

    UIRenderingPass(UIRenderingPass &&o) noexcept;

    UIRenderingPass &operator=(UIRenderingPass &&o) = delete;

    static UIRenderingPass
    Create(const VulkanContext &context, uint32_t width, uint32_t height);

    void begin(const glm::mat4 &viewMat);

    void end();

    void resizeAttachments(uint32_t width, uint32_t height);

    void
    drawUI(const VulkanBuffer &vertexBuffer, const VulkanBuffer &indexBuffer, uint32_t indexCount, uint32_t indexOffset,
           const glm::mat4 &modelMat, const VulkanMaterialInstance &material);

    inline const VulkanRenderPass &getOpaquePass() const { return *opaquePass; }

    inline const VulkanFramebuffer &getFramebuffer() const { return *framebuffer; }

private:
    void createAttachments(uint32_t width, uint32_t height);

    void updateUniformBuffer(const glm::mat4 &viewMat, const glm::uvec2 &viewportDimensions);

    void createStandardPipeline();

private:
    const VulkanContext &context;
    std::unique_ptr<VulkanRenderPass> opaquePass;
    std::unique_ptr<VulkanFramebuffer> framebuffer;

    // Dynamic resources ------------------------------------------------------
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;
    std::unique_ptr<VulkanDescriptorSetLayout> canvasDescriptorLayout;
    std::unique_ptr<VulkanDescriptorSetLayout> materialDescriptorLayout;
    std::unique_ptr<VulkanPipeline> pipeline;

    // Per Frame resources ----------------------------------------------------
    std::vector<VulkanDescriptorSet> perFrameDescriptorSets;
    std::vector<VulkanUniformBuffer> perFrameUniformBuffers;
    std::unique_ptr<Renderer::UniformBufferContent<CanvasUbo>> uboContent;

    // State resources --------------------------------------------------------
    glm::uvec2 viewportSize{0, 0};
};

