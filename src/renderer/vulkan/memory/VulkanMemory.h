#pragma once

#include <vulkan/vulkan.h>
#include <src/renderer/vulkan/command/VulkanCommandPool.h>

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanBuffer.h"

// TODO: This class needs to be refactored as soon as the Khronos Vulkan memory manager is added
class VulkanMemory {
public:
    VulkanMemory(const VulkanDevice &device, const VulkanCommandPool &commandPool);

    ~VulkanMemory() = default;

    VulkanMemory(const VulkanMemory &o) = delete;

    VulkanMemory &operator=(const VulkanMemory &o) = delete;

    VulkanMemory(VulkanMemory &&o) noexcept;

    VulkanMemory &operator=(VulkanMemory &&o) = delete;

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
    const VulkanDevice &device;
    const VulkanCommandPool &commandPool;
};

