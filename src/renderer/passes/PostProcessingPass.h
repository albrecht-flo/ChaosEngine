#pragma once

#include "src/core/Components.h"
#include "src/renderer/data/RenderObject.h"
#include "src/renderer/vulkan/context/VulkanContext.h"
#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorPool.h"
#include "src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "src/renderer/vulkan/image/VulkanImage.h"
#include "src/renderer/vulkan/image/VulkanSampler.h"

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
    explicit PostProcessingPass(const VulkanContext &context) : context(context) {}

    void init(const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer);

public:
    ~PostProcessingPass() = default;

    PostProcessingPass(const PostProcessingPass &o) = delete;

    PostProcessingPass &operator=(const PostProcessingPass &o) = delete;

    PostProcessingPass(PostProcessingPass &&o) noexcept;

    PostProcessingPass &operator=(PostProcessingPass &&o) = delete;

    static PostProcessingPass
    Create(const VulkanContext &context, const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer);

    void draw();

    void resizeAttachments(const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer);

    void updateConfiguration(const PostProcessingConfiguration &configuration);

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
    UniformBufferContent<ShaderConfig> uboContent;

};



