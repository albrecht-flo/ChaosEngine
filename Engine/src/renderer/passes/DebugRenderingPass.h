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

class DebugRenderingPass {
private:
    /// Canvas uniform object. Alignas 16 to ensure proper alignment with the GPU storage
    struct CameraUbo {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };
private:
    explicit DebugRenderingPass(const VulkanContext &context, const VulkanRenderPass& pass)
            : context(context), renderPass(pass) {}

    void init(uint32_t width, uint32_t height);

public:
    ~DebugRenderingPass() = default;

    DebugRenderingPass(const DebugRenderingPass &o) = delete;

    DebugRenderingPass &operator=(const DebugRenderingPass &o) = delete;

    DebugRenderingPass(DebugRenderingPass &&o) noexcept;

    DebugRenderingPass &operator=(DebugRenderingPass &&o) = delete;

    static DebugRenderingPass
    Create(const VulkanContext &context, const VulkanRenderPass &pass, uint32_t width, uint32_t height);

    void begin(const glm::mat4 &viewMat, const CameraComponent &camera);


    void resizeAttachments(uint32_t width, uint32_t height);

    void drawLines(const VulkanBuffer &vertexBuffer, uint32_t vertexCount);

private:

    void updateUniformBuffer(const glm::mat4 &viewMat, const CameraComponent &camera,
                                              const glm::uvec2 &viewportDimensions);

    void createStandardPipeline();

private:
    const VulkanContext &context;
    const VulkanRenderPass& renderPass;

    // Dynamic resources ------------------------------------------------------
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;
    std::unique_ptr<VulkanDescriptorSetLayout> cameraDescriptorLayout;
    std::unique_ptr<VulkanPipeline> pipeline;

    // Per Frame resources ----------------------------------------------------
    std::vector<VulkanDescriptorSet> perFrameDescriptorSets;
    std::vector<VulkanUniformBuffer> perFrameUniformBuffers;
    std::unique_ptr<Renderer::UniformBufferContent<CameraUbo>> uboContent;

    // State resources --------------------------------------------------------
    glm::uvec2 viewportSize{0, 0};
};

