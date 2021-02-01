#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <array>

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "src/renderer/vulkan/context/VulkanSwapChain.h"
#include "src/renderer/vulkan/image/VulkanImage.h"
#include "src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptor.h"
#include "src/renderer/data/Mesh.h"
#include "src/renderer/vulkan/image/VulkanTexture.h"
#include "src/renderer/data/RenderObject.h"

class VulkanRenderPassOld {
public:
    VulkanRenderPassOld(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain);

    ~VulkanRenderPassOld() = default;

    virtual void init() = 0;

    virtual void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) = 0;

    virtual void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) = 0;

    virtual void cmdEnd(VkCommandBuffer &cmdBuf) = 0;

    virtual void recreate() = 0;

    virtual void destroy() = 0;

    virtual void destroySwapChainDependent() = 0;

    [[nodiscard]] inline VkRenderPass vk() const { return renderPass; }

protected:
    // The common pointers to the vulkan context objects
    VulkanDevice &device;
    VulkanMemory &vulkanMemory;
    VulkanSwapChain &swapChain;

    // This vulkan render rendering
    VkRenderPass renderPass = {};

};
