#include "VulkanFramebuffer.h"

#include "src/renderer/vulkan/context/VulkanDevice.h"

#include <stdexcept>

/* Create framebuffers for swapchain image views */
VulkanFramebuffer VulkanFramebuffer::createFramebuffer(
        const VulkanDevice &device,
        std::vector<VkImageView> attachmentImages,
        VkRenderPass renderPass, uint32_t width, uint32_t height) {


    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass; // compatible renderpass (attachmentImages match)
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentImages.size());
    framebufferInfo.pAttachments = attachmentImages.data();
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
        : device(o.device), framebuffer(std::exchange(o.framebuffer, nullptr)) {}

VulkanFramebuffer::~VulkanFramebuffer() { destroy(); }


VulkanFramebuffer &VulkanFramebuffer::operator=(VulkanFramebuffer &&o) noexcept {
    if (this == &o)
        return *this;
    destroy();
    framebuffer = std::exchange(o.framebuffer, nullptr);
    return *this;
}

void VulkanFramebuffer::destroy() {
    vkDestroyFramebuffer(device.vk(), framebuffer, nullptr);
}

