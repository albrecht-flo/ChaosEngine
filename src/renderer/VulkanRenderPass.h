#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <array>

#include "general/VulkanDevice.h"
#include "general/VulkanSwapChain.h"
#include "image/VulkanImage.h"
#include "pipeline/VulkanPipeline.h"
#include "pipeline/VulkanDescriptor.h"
#include "data/Mesh.h"
#include "data/VulkanTexture.h"
#include "data/RenderObject.h"

class VulkanRenderPass {
public:
    VulkanRenderPass(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain);

    ~VulkanRenderPass() = default;

    virtual void init() = 0;

    virtual void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) = 0;

    virtual void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) = 0;

    virtual void cmdEnd(VkCommandBuffer &cmdBuf) = 0;

    virtual void recreate() = 0;

    virtual void destroy() = 0;

    virtual void destroySwapChainDependent() = 0;

    VkRenderPass getVkRenderPass() const { return renderPass; }

protected:
    // The common pointers to the vulkan context objects
    VulkanDevice &device;
    VulkanMemory &vulkanMemory;
    VulkanSwapChain &swapChain;

    // This vulkan render pass
    VkRenderPass renderPass = {};

};
