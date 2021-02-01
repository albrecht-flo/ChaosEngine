#pragma once

#include <vulkan/vulkan.h>

class VulkanDevice;

class VulkanImageView {
private:
    VulkanImageView(const VulkanDevice &device, VkImageView imageView) : device(device), imageView(imageView) {}

    void destroy();

public:
    [[deprecated]] explicit VulkanImageView(const VulkanDevice &device) : device(device), imageView(nullptr) {}

    ~VulkanImageView();

    VulkanImageView(const VulkanImageView &o) = delete;

    VulkanImageView &operator=(const VulkanImageView &o) = delete;

    VulkanImageView(VulkanImageView &&o) noexcept;

    VulkanImageView &operator=(VulkanImageView &&o) noexcept;

    static VulkanImageView
    Create(const VulkanDevice &device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    [[nodiscard]] inline VkImageView vk() const { return imageView; }

private:
    const VulkanDevice &device;
    VkImageView imageView;
};