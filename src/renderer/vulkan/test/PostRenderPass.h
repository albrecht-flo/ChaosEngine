#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <array>

#include "VulkanRenderPassOld.h"
#include "src/renderer/vulkan/pipeline/VulkanVertexInput.h"
#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "src/renderer/vulkan/image/VulkanTexture.h"
#include "src/renderer/vulkan/image/VulkanSampler.h"

class PostRenderPass : public VulkanRenderPassOld {
public:
    PostRenderPass(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain);

    ~PostRenderPass() = default;

    void
    setImageBufferViews(VkImageView newFramebufferView, VkImageView newDepthBufferView, VkImageView imGuiImageView);

    void updateCamera(const Camera &newCamera) { camera = newCamera; }

    void init() override;

    void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer, uint32_t viewportWidth,
                  uint32_t viewportHeight) override;

    void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) override;

    void cmdEnd(VkCommandBuffer &cmdBuf) override;

    void recreate() override;

    void destroy() override;

    void destroySwapChainDependent() override;

    [[nodiscard]] inline VkRenderPass vk() const { return renderPass->vk(); }

private:
    void createPipelineAndDescriptors();

private:
    // Pipelines
    std::unique_ptr<VulkanRenderPass> renderPass;
    std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayout;
    std::unique_ptr<VulkanPipeline> postprocessingPipeline;
    VulkanVertexInput vertex_3P_3C_3N_2U;

    VkImageView framebufferView{};
    VulkanSampler framebufferSampler;
    VkImageView depthBufferView{};
    VulkanSampler depthBufferSampler;
    VkImageView imGuiImageView{};
    VulkanSampler imGuiImageSampler;
    // The objects for uniform buffer linking
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;
    std::unique_ptr<VulkanDescriptorSet> descriptorSet;

    Camera camera{};
    VulkanTexture backgroundTexture;
};

