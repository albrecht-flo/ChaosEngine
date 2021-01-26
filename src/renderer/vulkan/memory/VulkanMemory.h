#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanBuffer.h"

class VulkanMemory {
public:
   explicit VulkanMemory(VulkanDevice &device);
    ~VulkanMemory() = default;

    void init(VkCommandPool cmdPool);

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
    VkCommandPool m_commandPool;
};

