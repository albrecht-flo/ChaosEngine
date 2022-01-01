#include "VulkanMemory.h"

#include "Engine/src/renderer/vulkan/context/VulkanContext.h"
#include "VulkanBuffer.h"
#include "Engine/src/renderer/vulkan/image/VulkanImage.h"

#include <stdexcept>
#include <cstring>

VulkanMemory VulkanMemory::Create(const VulkanDevice &device, const VulkanInstance &instance,
                                  const VulkanCommandPool &commandPool) {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorInfo.physicalDevice = device.getPhysicalDevice();
    allocatorInfo.device = device.vk();
    allocatorInfo.instance = instance.vk();

    VmaAllocator allocator;
    if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS) {
        throw std::runtime_error("[VMA] Failed to create VulkanMemoryAllocator!");
    }

    return VulkanMemory(device, commandPool, allocator);
}

VulkanMemory::VulkanMemory(const VulkanDevice &device, const VulkanCommandPool &commandPool, VmaAllocator allocator)
        : device(device), commandPool(commandPool), allocator(allocator) {}

VulkanMemory::VulkanMemory(VulkanMemory &&o) noexcept
        : device(o.device), commandPool(o.commandPool), allocator(std::exchange(o.allocator, nullptr)) {}


/* Creates a new buffer with dedicated device memory ! bad !
	Optimal should be few big device memory regions and buffers into these regions */
VulkanBuffer
VulkanMemory::createBuffer(VkDeviceSize size, VkBufferUsageFlagBits bufferUsage, VmaMemoryUsage memoryUsage) const {
    // Create buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = bufferUsage;

    std::array<uint32_t, 2> queueFamilyIndices = {
            device.getGraphicsQueueFamilyIndex(),
            device.getTransferQueueFamilyIndex()
    };
    if (device.getGraphicsQueueFamilyIndex() != device.getTransferQueueFamilyIndex()) {
        // We copy on the transfer queue and use it on the graphics queue
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsage;

    VkBuffer buffer{};
    VmaAllocation allocation{};
    if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("[VMA] Failed to create buffer!");
    }

    return VulkanBuffer{*this, buffer, allocation};
}

/* Copies the contents of a source buffer to a destination buffer on the GPU. */
void VulkanMemory::copyBuffer(const VulkanBuffer &srcBuffer, const VulkanBuffer &dstBuffer, VkDeviceSize size) const {
    commandPool.runInSingeTimeCommandBuffer([&](VkCommandBuffer commandBuffer) {
        // Put copy cmd
        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        // copyRegion.srcOffset = 0;
        // copyRegion.dstOffset = 0;
        vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);
    });


}

void VulkanMemory::copyDataToBuffer(const VulkanBuffer &buffer, const void *data, size_t size,
                                    size_t offset) const {
    void *bufferData;
    vmaMapMemory(allocator, buffer.allocation, &bufferData);
    memcpy(&(reinterpret_cast<char *>(bufferData)[offset]), data, size);
    vmaUnmapMemory(allocator, buffer.allocation);
}

void VulkanMemory::copyBufferToImage(const VulkanBuffer &buffer, const VulkanImage &image, uint32_t width,
                                     uint32_t height) const {
    commandPool.runInSingeTimeCommandBuffer([&](VkCommandBuffer commandBuffer) {
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer.vk(), image.vk(),
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    });
}

VulkanUniformBuffer
VulkanMemory::createUniformBuffer(uint32_t elementSize, uint32_t count, bool aligned) const {
    VkDeviceSize uboSize = (long) elementSize * count;
    VkDeviceSize alignment = 0;
    if (aligned) { // Align the data
        alignment = device.getProperties().limits.minUniformBufferOffsetAlignment;
        uboSize = (elementSize < alignment) ? alignment : (elementSize + (elementSize % alignment));
    }

    VkDeviceSize bufferSize = uboSize * count;

    VulkanBuffer buffer = createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    return VulkanUniformBuffer{std::move(buffer), uboSize, alignment};
}

/* Creates a buffer containing 'size' bytes from 'data'.
	The buffer is a transfer_dst and device_local.
	The data is transmitted using a staging buffer. 
	*/
VulkanBuffer VulkanMemory::createInputBuffer(VkDeviceSize size, const void *data, VkBufferUsageFlags flags) const {
    // Staging buffer to contain data for transfer
    // Creates buffer with usage=transfer_src, host visible and coherent meaning the cpu has access to the memory and
    // changes are immediately known to the driver which will transfer the memmory before the next vkQueueSubmit
    VulkanBuffer stagingBuffer = createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    copyDataToBuffer(stagingBuffer, data, size, 0);

    // Vertex buffer
    // Create vertex buffer, usage=transfer_dst | vertexbuffer, it is device local
    VulkanBuffer inputBuffer = createBuffer(size,
                                            static_cast<VkBufferUsageFlagBits>(flags |
                                                                               VK_BUFFER_USAGE_TRANSFER_DST_BIT),
                                            VMA_MEMORY_USAGE_GPU_ONLY);

    // Create copy operation to move data from staging into vertex buffer
    copyBuffer(stagingBuffer, inputBuffer, size);

    return inputBuffer;
}

VulkanImage VulkanMemory::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                                      VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, bool dedicatedAllocation)
const {
    // Create the image
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1; // 1 layer -> 2D
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format; // must be corresponding to the data
    imageInfo.tiling = tiling; // usage optimized layout
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage; // we need to copy to it and sample from the shader
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // multisample
    imageInfo.flags = 0;

    std::array<uint32_t, 2> queueFamilyIndices = {
            device.getGraphicsQueueFamilyIndex(),
            device.getTransferQueueFamilyIndex()
    };
    if (device.getGraphicsQueueFamilyIndex() != device.getTransferQueueFamilyIndex()) {
        // We copy on the transfer queue and use it on the graphics queue
        imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        imageInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsage;
    if (dedicatedAllocation)
        allocInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    VkImage image{};
    VmaAllocation allocation{};
    if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("[VMA] Failed to create Image!");
    }

    return VulkanImage{*this, image, allocation, static_cast<uint32_t>(width), static_cast<uint32_t>(height), format};
}

// --------------------------------- Resource Destruction --------------------------------------------------------------

void VulkanMemory::destroyImage(VkImage image, VmaAllocation imageAllocation) const {
    vmaDestroyImage(allocator, image, imageAllocation);
}

void VulkanMemory::destroyBuffer(VkBuffer buffer, VmaAllocation bufferAllocation) const {
    vmaDestroyBuffer(allocator, buffer, bufferAllocation);
}

// ------------------------------------- Helpers -----------------------------------------------------------------------

uint32_t VulkanMemory::sizeWithUboPadding(size_t originalSize) const {
    // Source https://github.com/SaschaWillems/Vulkan/tree/master/examples/dynamicuniformbuffer
    size_t minUboAlignment = device.getProperties().limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = originalSize;
    if (minUboAlignment > 0) {
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    return static_cast<uint32_t>(alignedSize);
}
