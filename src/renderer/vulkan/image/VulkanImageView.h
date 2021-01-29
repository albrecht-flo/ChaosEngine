#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "src/renderer/vulkan/context/VulkanDevice.h"

class VulkanImageView {
public:
    static VkImageView create(const VulkanDevice &device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    static void destroy(const VulkanDevice &device, VkImageView imageView);
};