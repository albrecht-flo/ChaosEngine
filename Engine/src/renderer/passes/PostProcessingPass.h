#pragma once

#include "Engine/src/core/Components.h"
#include "Engine/src/renderer/data/RenderObject.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPool.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "Engine/src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "Engine/src/renderer/vulkan/image/VulkanImage.h"
#include "Engine/src/renderer/vulkan/image/VulkanSampler.h"

#include <array>

class PostProcessingPass {
public:
    struct PostProcessingConfiguration {
        CameraComponent camera;
    };

private:
    struct ShaderConfig {
        float cameraNear = 0; // initialize to invalid default to ensure shader gets configured
        float cameraFar = 0; // initialize to invalid default to ensure shader gets configured

        bool operator==(const ShaderConfig &o) const {
            return cameraNear == o.cameraNear &&
                   cameraFar == o.cameraFar;
        }
    };

private:
    explicit PostProcessingPass(const VulkanContext &context, bool renderToSwapchain)
            : context(context), renderToSwapchain(renderToSwapchain) {}

    void
    init(const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer, uint32_t width, uint32_t height);

public:
    ~PostProcessingPass() = default;

    PostProcessingPass(const PostProcessingPass &o) = delete;

    PostProcessingPass &operator=(const PostProcessingPass &o) = delete;

    PostProcessingPass(PostProcessingPass &&o) noexcept;

    PostProcessingPass &operator=(PostProcessingPass &&o) = delete;

    static PostProcessingPass
    Create(const VulkanContext &context, const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer,
           bool renderToSwapchain = true, uint32_t width = 0, uint32_t height = 0);

    void draw();

    void resizeAttachments(const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer,
                           uint32_t width = 0, uint32_t height = 0);

    void updateConfiguration(const PostProcessingConfiguration &configuration);

    const VulkanImageBuffer &getColorAttachment() const {
        assert("Color Attachment can only be retrieved if rendering to buffer." && colorAttachmentBuffer != nullptr);
        return *colorAttachmentBuffer;
    }

private:
    void writeDescriptorSet(const VulkanImageView &colorView, const VulkanImageView &depthView);


private:
    const VulkanContext &context;
    bool renderToSwapchain;
    std::unique_ptr<VulkanRenderPass> renderPass;
    std::unique_ptr<VulkanImageBuffer> colorAttachmentBuffer;
    std::vector<VulkanFramebuffer> swapChainFrameBuffers;

    std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayout;
    std::unique_ptr<VulkanPipeline> postprocessingPipeline;
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;
    std::unique_ptr<VulkanBuffer> quadBuffer;
    std::unique_ptr<VulkanSampler> colorBufferSampler;
    std::unique_ptr<VulkanSampler> depthBufferSampler;

    std::unique_ptr<VulkanDescriptorSet> perFrameDescriptorSet;
    std::unique_ptr<VulkanUniformBuffer> perFrameUniformBuffer;
    UniformBufferContent<ShaderConfig> uboContent;

};



