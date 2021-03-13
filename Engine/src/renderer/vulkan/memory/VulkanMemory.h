#pragma once

#include <vulkan/vulkan.h>
#include <Engine/src/renderer/vulkan/command/VulkanCommandPool.h>

#include "VulkanBuffer.h"

class VulkanDevice;

// TODO: This class needs to be refactored as soon as the Khronos Vulkan memory manager is added
class VulkanMemory {
public:
    VulkanMemory(const VulkanDevice &device, const VulkanCommandPool &commandPool);

    ~VulkanMemory() = default;

    VulkanMemory(const VulkanMemory &o) = delete;

    VulkanMemory &operator=(const VulkanMemory &o) = delete;

    VulkanMemory(VulkanMemory &&o) noexcept;

    VulkanMemory &operator=(VulkanMemory &&o) = delete;

    void destroy(VulkanBuffer buffer) const;

    VulkanBuffer createInputBuffer(VkDeviceSize size, const void *data, VkBufferUsageFlags flags) const;

    [[nodiscard]] const VulkanUniformBuffer
    createUniformBuffer(uint32_t elementSize, VkBufferCreateFlags flags, uint32_t count = 1, bool aligned = false) const;

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory) const;

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

    void copyDataToBuffer(VkBuffer buffer, VkDeviceMemory memory, const void *data, size_t size, size_t offset = 0) const;

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;

//private: for now
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    // TOBE moved
    VkCommandBuffer beginSingleTimeCommands() const;

    void endSingleTimeCommands(VkCommandBuffer &commandBuffer) const;

private:
    const VulkanDevice &device;
    const VulkanCommandPool &commandPool;
};

