#include "VulkanImageView.h"

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"

#include <stdexcept>

/* Creates an image view for an image. */
VulkanImageView
VulkanImageView::Create(const VulkanDevice &device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView imageView = {};
    if (vkCreateImageView(device.vk(), &createInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image views!");
    }
    return VulkanImageView{device, imageView};
}

VulkanImageView::VulkanImageView(VulkanImageView &&o) noexcept
        : device(o.device), imageView(std::exchange(o.imageView, nullptr)) {}

VulkanImageView::~VulkanImageView() {
    destroy();
}

VulkanImageView &VulkanImageView::operator=(VulkanImageView &&o) noexcept {
    if (this == &o)
        return *this;

    destroy();
    imageView = std::exchange(o.imageView, nullptr);
    return *this;
}

void VulkanImageView::destroy() {
    if (imageView != nullptr)
        vkDestroyImageView(device.vk(), imageView, nullptr);
}