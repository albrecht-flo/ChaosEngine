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

class ImGuiRenderPass : public VulkanRenderPass {
public:
    ImGuiRenderPass() = default;

    ImGuiRenderPass(VulkanDevice *device, VulkanMemory *vulkanMemory, VulkanSwapChain *swapChain, Window *window);

    ~ImGuiRenderPass() = default;

    virtual void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) override;

    virtual void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) override;

    virtual void cmdEnd(VkCommandBuffer &cmdBuf) override;

    virtual void recreate() override;

    virtual void destroy() override;

    virtual void destroySwapChainDependent() override;

private:
    void createRenderPass();

private:
    // The objects for uniform buffer linking
    VkDescriptorPool m_descriptorPool = {};
    Window *m_window;
};

