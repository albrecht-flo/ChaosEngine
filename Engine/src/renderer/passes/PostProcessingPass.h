#pragma once

#include "Engine/src/core/Components.h"
#include "Engine/src/renderer/api/RenderMesh.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPool.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "Engine/src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "Engine/src/renderer/vulkan/image/VulkanImage.h"
#include "Engine/src/renderer/vulkan/image/VulkanSampler.h"

#include <memory>
#include <vector>

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
    init(uint32_t width, uint32_t height, const VulkanFramebuffer &previousPassSceneFB,
         const VulkanFramebuffer &previousPassUIFB, const VulkanFramebuffer &previousPassTextFB);

public:
    ~PostProcessingPass() = default;

    PostProcessingPass(const PostProcessingPass &o) = delete;

    PostProcessingPass &operator=(const PostProcessingPass &o) = delete;

    PostProcessingPass(PostProcessingPass &&o) noexcept;

    PostProcessingPass &operator=(PostProcessingPass &&o) = delete;

    static PostProcessingPass
    Create(const VulkanContext &context,
           const VulkanFramebuffer &previousPassSceneFB, const VulkanFramebuffer &previousPassUIFB,
           const VulkanFramebuffer &previousPassTextFB, bool renderToSwapchain, uint32_t width, uint32_t height);

    void draw();

    void resizeAttachments(const VulkanFramebuffer &sceneFramebuffer, const VulkanFramebuffer &uiFramebuffer,
                           const VulkanFramebuffer &textFramebuffer, uint32_t width, uint32_t height);

    void updateConfiguration(const PostProcessingConfiguration &configuration);

    [[nodiscard]] const VulkanFramebuffer &getColorAttachment() const {
        assert("Color Attachment can only be retrieved if rendering to buffer." && !renderToSwapchain);
        return swapChainFrameBuffers[0];
    }

private:
    void writeDescriptorSet(const VulkanFramebuffer &previousPassSceneFB, const VulkanFramebuffer &previousPassUIFB,
                            const VulkanFramebuffer &previousPassTextFB);

private:
    const VulkanContext &context;
    bool renderToSwapchain;
    std::unique_ptr<VulkanRenderPass> renderPass;
    std::vector<VulkanFramebuffer> swapChainFrameBuffers; // [0] is final color attachment if not rendering to swapchain

    std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayout;
    std::unique_ptr<VulkanPipeline> postprocessingPipeline;
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;
    std::unique_ptr<VulkanBuffer> quadBuffer;

    std::unique_ptr<VulkanDescriptorSet> perFrameDescriptorSet;
    std::unique_ptr<VulkanUniformBuffer> perFrameUniformBuffer;
    std::unique_ptr<Renderer::UniformBufferContent<ShaderConfig>> uboContent;

    // State resources --------------------------------------------------------
    glm::uvec2 viewportSize{0, 0};

    void createAttachments(uint32_t width, uint32_t height);
};



