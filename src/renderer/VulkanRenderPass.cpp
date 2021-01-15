#include "VulkanRenderPass.h"

/* Configures the render pass with the attachments and subpasses */
VulkanRenderPass::VulkanRenderPass(VulkanDevice *device, VulkanMemory *vulkanMemory, VulkanSwapChain *swapChain) :
        m_device(device), m_vulkanMemory(vulkanMemory), m_swapChain(swapChain) {
    // init needs to be implemented by subclasses
}
