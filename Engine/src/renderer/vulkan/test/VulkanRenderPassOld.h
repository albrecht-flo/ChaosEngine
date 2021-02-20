#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <array>

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"
#include "Engine/src/renderer/vulkan/context/VulkanSwapChain.h"
#include "Engine/src/renderer/vulkan/memory/VulkanMemory.h"
#include "Engine/src/renderer/data/RenderObject.h"

class VulkanRenderPassOld {
public:
    VulkanRenderPassOld(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain);

    ~VulkanRenderPassOld() = default;

    virtual void init() = 0;

    virtual void
    cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer, uint32_t viewportWidth = 0,
             uint32_t viewportHeight = 0) = 0;

    virtual void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) = 0;

    virtual void cmdEnd(VkCommandBuffer &cmdBuf) = 0;

    virtual void recreate() = 0;

    virtual void destroy() = 0;

    virtual void destroySwapChainDependent() = 0;

protected:
    // The common pointers to the vulkan context objects
    VulkanDevice &device;
    VulkanMemory &vulkanMemory;
    VulkanSwapChain &swapChain;
};
