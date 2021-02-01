#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "src/renderer/vulkan/memory/VulkanMemory.h"

// TODO: massive refactor to be a RAII Wrapper + Builder
class VulkanImage {
private:
    VulkanImage(const VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory);

public:
    static VkImage createFromFile(const VulkanDevice &device, VulkanMemory &vulkanMemory, const std::string &filename,
                                  VkDeviceMemory &imageMemory/*TEMP*/);

    static VkImage
    createRawImage(const VulkanDevice &device, VulkanMemory &vulkanMemory, uint32_t width, uint32_t height, VkFormat format,
                   VkDeviceMemory &imageMemory/*TEMP*/);

    static VkImage
    createDepthBufferImage(const VulkanDevice &device, const VulkanMemory &vulkanMemory, uint32_t width, uint32_t height,
                           VkFormat depthFormat, VkDeviceMemory &depthImageMemory/*TEMP*/);

    static void destroy(const VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory);

    static VkFormat getDepthFormat(const VulkanDevice &device);

private:
    static void
    createImage(const VulkanDevice &device, const VulkanMemory &vulkanMemory, uint32_t widht, uint32_t height, VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image,
                VkDeviceMemory &imageMemory);

    static void
    transitionImageLayout(VulkanMemory &vulkanMemory, VkImage image, VkFormat format, VkImageLayout oldLayout,
                          VkImageLayout newLayout);

    static bool hasStencilComponent(VkFormat format);
};

