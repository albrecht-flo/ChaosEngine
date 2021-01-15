#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <vector>

#include "../general/VulkanDevice.h"

class VulkanFramebuffer {
public:
    static VkFramebuffer createFramebuffer(VulkanDevice &device,
                                           std::vector<VkImageView> attachments,
                                           VkRenderPass renderPass, uint32_t width, uint32_t height);

    static void destroy(VulkanDevice &device, VkFramebuffer framebuffer);
};