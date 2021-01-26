#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "src/renderer/vulkan/context/VulkanDevice.h"

class VulkanSampler {
public:
    static VkSampler create(VulkanDevice &device, VkFilter filter = VK_FILTER_LINEAR);

    static void destroy(VulkanDevice &device, VkSampler sampler);
};

