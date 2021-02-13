#pragma once

#include "src/renderer/vulkan/context/VulkanContext.h"
#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "src/renderer/vulkan/image/VulkanImage.h"
#include "src/renderer/vulkan/image/VulkanSampler.h"

#include <array>

class PostProcessingPass {
public:
    struct RenderPassConfiguration {
        float cameraNear;
        float cameraFar;
    };
private:
    explicit PostProcessingPass(const VulkanContext &context) : context(context) {}

    void init(const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer);

public:
    ~PostProcessingPass() = default;

    PostProcessingPass(const PostProcessingPass &o) = delete;

    PostProcessingPass &operator=(const PostProcessingPass &o) = delete;

    PostProcessingPass(PostProcessingPass &&o) noexcept;

    PostProcessingPass &operator=(PostProcessingPass &&o) = delete;

    static PostProcessingPass
    Create(const VulkanContext &context, const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer,
           const RenderPassConfiguration &configuration);

    void draw();

    void resizeAttachments(const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer);

    void updateConfiguration(const RenderPassConfiguration &configuration);

private:
    void writeDescriptorSet(const VulkanImageView &colorView, const VulkanImageView &depthView);

private:
    const VulkanContext &context;
    std::unique_ptr<VulkanRenderPass> renderPass;
    std::vector<VulkanFramebuffer> swapChainFrameBuffers;

    std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayout;
    std::unique_ptr<VulkanPipeline> postprocessingPipeline;
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;
    std::unique_ptr<VulkanBuffer> quadBuffer;
    std::unique_ptr<VulkanSampler> colorBufferSampler;
    std::unique_ptr<VulkanSampler> depthBufferSampler;

    std::unique_ptr<VulkanDescriptorSet> perFrameDescriptorSet;
    std::unique_ptr<VulkanUniformBuffer> perFrameUniformBuffer;
    UniformBufferContent<RenderPassConfiguration> uboContent;

};



