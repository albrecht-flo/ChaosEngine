#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <string>

#include "../general/VulkanDevice.h"
#include "../memory/VulkanMemory.h"

class VulkanTexture {
public:
    VulkanTexture();

    VulkanTexture(VkImage image, VkDeviceMemory imageMemory, VkImageView imageView, VkSampler sampler);

    ~VulkanTexture();

public:
    static VulkanTexture createTexture(VulkanDevice &device, VulkanMemory &vulkanMemroy, const std::string &filename);

    static void destroy(VulkanDevice &device, VulkanTexture &texture);

public:
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE; // TEMP
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
};