#pragma once

#include <vulkan/vulkan.h>
#include "src/renderer/vulkan/context/VulkanDevice.h"

class VulkanSampler {
public:
    static VkSampler create(const VulkanDevice &device, VkFilter filter = VK_FILTER_LINEAR);

    static void destroy(const VulkanDevice &device, VkSampler sampler);
};

