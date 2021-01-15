#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "../general/VulkanDevice.h"

class VulkanImageView {
public:
    static VkImageView create(VulkanDevice &device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    static void destroy(VulkanDevice &device, VkImageView imageView);
};