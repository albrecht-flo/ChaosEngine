#include "VulkanFramebuffer.h"

#include <stdexcept>

/* Create framebuffers for swapchain image views */
VulkanFramebuffer VulkanFramebuffer::createFramebuffer(
        const VulkanDevice &device,
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
    if (vkCreateFramebuffer(device.vk(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create framebuffer!");
    }

    return VulkanFramebuffer{device, framebuffer};
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanFramebuffer::VulkanFramebuffer(const VulkanDevice &device, VkFramebuffer frameBuffer)
        : device(device), framebuffer(frameBuffer) {}

VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer &&o) noexcept
        : device(o.device), framebuffer(o.framebuffer) { o.framebuffer = VK_NULL_HANDLE; }

VulkanFramebuffer::~VulkanFramebuffer() { destroy(); }


VulkanFramebuffer &VulkanFramebuffer::operator=(VulkanFramebuffer &&o) noexcept {
    if (this == &o)
        return *this;
    destroy();
    framebuffer = o.framebuffer;
    o.framebuffer = VK_NULL_HANDLE;
    return *this;
}

void VulkanFramebuffer::destroy() {
    if (framebuffer != VK_NULL_HANDLE)
        vkDestroyFramebuffer(device.vk(), framebuffer, nullptr);
}

