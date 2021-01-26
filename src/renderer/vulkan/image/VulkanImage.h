#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <string>

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "src/renderer/vulkan/memory/VulkanMemory.h"

class VulkanImage {
private:
    VulkanImage(VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory);

public:
    static VkImage createFromFile(VulkanDevice &device, VulkanMemory &vulkanMemory, const std::string &filename,
                                  VkDeviceMemory &imageMemory/*TEMP*/);

    static VkImage
    createRawImage(VulkanDevice &device, VulkanMemory &vulkanMemory, uint32_t width, uint32_t height, VkFormat format,
                   VkDeviceMemory &imageMemory/*TEMP*/);

    static VkImage
    createDepthBufferImage(VulkanDevice &device, VulkanMemory &vulkanMemory, uint32_t width, uint32_t height,
                           VkFormat depthFormat, VkDeviceMemory &depthImageMemory/*TEMP*/);

    static void destroy(VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory);

    static VkFormat getDepthFormat(VulkanDevice &device);

private:
    static void
    createImage(VulkanDevice &device, VulkanMemory &vulkanMemory, uint32_t widht, uint32_t height, VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image,
                VkDeviceMemory &imageMemory);

    static void
    transitionImageLayout(VulkanMemory &vulkanMemory, VkImage image, VkFormat format, VkImageLayout oldLayout,
                          VkImageLayout newLayout);

    static bool hasStencilComponent(VkFormat format);
};

