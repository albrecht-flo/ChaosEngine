#include "VulkanRenderPassOld.h"

/* Configures the render rendering with the attachments and subpasses */
VulkanRenderPassOld::VulkanRenderPassOld(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain) :
        device(device), vulkanMemory(vulkanMemory), swapChain(swapChain) {}
