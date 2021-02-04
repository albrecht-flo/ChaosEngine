#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <array>

#include "VulkanRenderPassOld.h"
#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "src/renderer/vulkan/context/VulkanSwapChain.h"
#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "src/renderer/vulkan/image/VulkanImage.h"
#include "src/renderer/vulkan/image/VulkanSampler.h"
#include "src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptor.h"
#include "src/renderer/data/Mesh.h"
#include "src/renderer/vulkan/image/VulkanTexture.h"
#include "src/renderer/data/RenderObject.h"

class PostRenderPass : public VulkanRenderPassOld {
public:
    PostRenderPass(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain);

    ~PostRenderPass() = default;

    void
    setImageBufferViews(VkImageView newFramebufferView, VkImageView newDepthBufferView, VkImageView imGuiImageView);

    void updateCamera(const Camera &newCamera) { camera = newCamera; }

    void init() override;

    void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) override;

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

    VkImageView framebufferView{};
    VkSampler framebufferSampler{};
    VkImageView depthBufferView{};
    VkSampler depthBufferSampler{};
    VkImageView imGuiImageView{};
    VkSampler imGuiImageSampler{};
    // The objects for uniform buffer linking
    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};

    Camera camera{};
    VulkanTexture backgroundTexture;
};

