#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <array>

#include "../VulkanRenderPass.h"
#include "../general/VulkanDevice.h"
#include "../general/VulkanSwapChain.h"
#include "../image/VulkanImage.h"
#include "../image/VulkanSampler.h"
#include "../pipeline/VulkanPipeline.h"
#include "../pipeline/VulkanDescriptor.h"
#include "../data/Mesh.h"
#include "../data/VulkanTexture.h"
#include "../data/RenderObject.h"

class PostRenderPass : public VulkanRenderPass {
public:
    PostRenderPass() = default;

    PostRenderPass(VulkanDevice *device, VulkanMemory *vulkanMemory, VulkanSwapChain *swapChain);

    ~PostRenderPass() = default;

    void setImageBufferViews(VkImageView framebufferView, VkImageView depthbufferView, VkImageView imGuiImageView);

    void updateCamera(Camera &camera) { m_camera = camera; }

    virtual void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) override;

    virtual void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) override;

    virtual void cmdEnd(VkCommandBuffer &cmdBuf) override;

    virtual void recreate() override;

    virtual void destroy() override;

    virtual void destroySwapChainDependent() override;

private:
    void createRenderPass();

    void createPipelineAndDescriptors();

private:
    // Pipelines
    DescriptorSetLayout m_descriptorSetLayout = {};
    PipelineLayout m_postprocessingPipelineLayout;
    VulkanPipeline m_postprocessingPipeline;

    VkImageView m_framebufferView;
    VkSampler m_framebufferSampler;
    VkImageView m_depthBufferView;
    VkSampler m_depthBufferSampler;
    VkImageView m_imGuiImageView;
    VkSampler m_imGuiImageSampler;
    // The objects for uniform buffer linking
    VkDescriptorPool m_descriptorPool = {};
    VkDescriptorSet m_descriptorSet;

    Camera m_camera;
    VulkanTexture m_backgroundTexture;
};

