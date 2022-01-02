#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>


class VulkanContext;

class VulkanDevice;

class VulkanInstance;

class VulkanCommandPool;

class VulkanBuffer;

class VulkanUniformBuffer;

class VulkanImage;

class VulkanMemory {
public:
    VulkanMemory(const VulkanDevice &device, const VulkanCommandPool &commandPool, VmaAllocator allocator);

    ~VulkanMemory();

    VulkanMemory(const VulkanMemory &o) = delete;

    VulkanMemory &operator=(const VulkanMemory &o) = delete;

    VulkanMemory(VulkanMemory &&o) noexcept;

    VulkanMemory &operator=(VulkanMemory &&o) = delete;

    static VulkanMemory
    Create(const VulkanDevice &context, const VulkanInstance &instance, const VulkanCommandPool &commandPool);

// ------------------------------------ Creation Methods ---------------------------------------------------------------

    VulkanBuffer createInputBuffer(VkDeviceSize size, const void *data, VkBufferUsageFlags flags) const;

    [[nodiscard]] VulkanUniformBuffer
    createUniformBuffer(uint32_t elementSize, uint32_t count, bool aligned) const;

    [[nodiscard]] VulkanBuffer
    createBuffer(VkDeviceSize size, VkBufferUsageFlagBits bufferUsage, VmaMemoryUsage memoryUsage) const;

    [[nodiscard]] VulkanImage
    createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, bool dedicatedAllocation = false) const;

// ------------------------------------ Update Methods -----------------------------------------------------------------
    void
    copyDataToBuffer(const VulkanBuffer &buffer, const void *data, size_t size, size_t offset = 0) const;

    void copyBufferToImage(const VulkanBuffer &buffer, const VulkanImage &image, uint32_t width, uint32_t height) const;

// ---------------------------------- Destruction Methods --------------------------------------------------------------

    void destroyImage(VkImage image, VmaAllocation imageAllocation) const;

    void destroyBuffer(VkBuffer buffer, VmaAllocation bufferAllocation) const;

// ------------------------------------- Helpers -----------------------------------------------------------------------

    /// Calculates required size with alignment based on minimum device offset alignment
    [[nodiscard]] uint32_t sizeWithUboPadding(size_t originalSize) const;

    [[nodiscard]] const VulkanCommandPool &getTransferCommandPool() const { return commandPool; }

private:
    void copyBuffer(const VulkanBuffer &srcBuffer, const VulkanBuffer &dstBuffer, VkDeviceSize size) const;

private:
    const VulkanDevice &device;
    const VulkanCommandPool &commandPool;
    VmaAllocator allocator;
};

