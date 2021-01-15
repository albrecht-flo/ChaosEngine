#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "../general/VulkanDevice.h"
#include "VulkanBuffer.h"

class VulkanMemory {
public:
    VulkanMemory(VulkanDevice &device, const VkCommandPool &cmdPool);

    ~VulkanMemory();

    void destroy();

    void destroy(VulkanBuffer buffer);

    const VulkanBuffer createInputBuffer(VkDeviceSize size, void *data, VkBufferUsageFlags flags);

    const VulkanUniformBuffer
    createUniformBuffer(uint32_t elementSize, VkBufferCreateFlags flags, uint32_t count = 1, bool aligned = false);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void copyDataToBuffer(VkBuffer buffer, VkDeviceMemory memory, const void *data, size_t size);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

//private: for now
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // TOBE moved
    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer &commandBuffer);

private:
    VulkanDevice &m_device;
    const VkCommandPool &m_commandPool;
};

