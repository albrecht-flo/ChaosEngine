#include "VulkanFramebuffer.h"

#include <stdexcept>

/* Create framebuffers for swapchain image views */
VkFramebuffer VulkanFramebuffer::createFramebuffer(
        VulkanDevice &device,
        std::vector<VkImageView> attachments,
        VkRenderPass renderPass, uint32_t width, uint32_t height) {


    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass; // compatible renderpass (attachments match)
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1; // single images

    VkFramebuffer framebuffer;
    if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create framebuffer!");
    }

    return framebuffer;
}

void VulkanFramebuffer::destroy(VulkanDevice &device, VkFramebuffer framebuffer) {
    vkDestroyFramebuffer(device.getDevice(), framebuffer, nullptr);
}