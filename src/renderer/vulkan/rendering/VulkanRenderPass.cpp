#include "VulkanRenderPass.h"

/* Configures the render rendering with the attachments and subpasses */
VulkanRenderPass::VulkanRenderPass(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain) :
        device(device), vulkanMemory(vulkanMemory), swapChain(swapChain) {}
