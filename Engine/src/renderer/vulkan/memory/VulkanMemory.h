#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "VulkanBuffer.h"

class VulkanInstance;

class VulkanDevice;

class VulkanCommandPool;

class VulkanMemory {
public:
    VulkanMemory(const VulkanDevice &device, const VulkanCommandPool &commandPool, VmaAllocator allocator);

    ~VulkanMemory() { if (allocator != nullptr) vmaDestroyAllocator(allocator); };

    VulkanMemory(const VulkanMemory &o) = delete;

    VulkanMemory &operator=(const VulkanMemory &o) = delete;

    VulkanMemory(VulkanMemory &&o) noexcept;

    VulkanMemory &operator=(VulkanMemory &&o) = delete;

    static VulkanMemory
    Create(const VulkanDevice &context, const VulkanInstance &instance, const VulkanCommandPool &commandPool);

    VulkanBuffer createInputBuffer(VkDeviceSize size, const void *data, VkBufferUsageFlags flags) const;

    [[nodiscard]] VulkanUniformBuffer
    createUniformBuffer(uint32_t elementSize, uint32_t count, bool aligned) const;

    [[nodiscard]] VulkanBuffer
    createBuffer(VkDeviceSize size, VkBufferUsageFlagBits bufferUsage, VmaMemoryUsage memoryUsage) const;

    void
    copyDataToBuffer(const VulkanBuffer &buffer, const void *data, size_t size, size_t offset = 0) const;

    void copyBufferToImage(const VulkanBuffer &buffer, VkImage image, uint32_t width, uint32_t height) const;

//private:  // TODO: Move Image creation here as well
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

private:
    void copyBuffer(const VulkanBuffer &srcBuffer, const VulkanBuffer &dstBuffer, VkDeviceSize size) const;

public:  // TODO: Move single time commands to other class
    VkCommandBuffer beginSingleTimeCommands() const;

    void endSingleTimeCommands(VkCommandBuffer &commandBuffer) const;

private:
    const VulkanDevice &device;
    const VulkanCommandPool &commandPool;
    VmaAllocator allocator;
};

