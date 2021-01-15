#include "VulkanMemory.h"

#include <stdexcept>
#include <cstring>

VulkanMemory::VulkanMemory(VulkanDevice &device, const VkCommandPool &cmdPool) :
        m_device(device), m_commandPool(cmdPool) {

}

VulkanMemory::~VulkanMemory() {

}

void VulkanMemory::destroy(VulkanBuffer buffer) {
    vkDestroyBuffer(m_device.getDevice(), buffer.buffer, nullptr);
    vkFreeMemory(m_device.getDevice(), buffer.memory, nullptr);
}

void VulkanMemory::destroy() {

}

/* Creates a new buffer with dedicated device memory ! bad !
	Optimal should be few big device memory regions and buffers into these regions */
void VulkanMemory::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    // Create buffer
    // Buffers only define the memory but need to be linked to the actuall memory
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size; // size in bytes
    bufferInfo.usage = usage; // the usage the buffer contents will be used for
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // exclusiv to the graphcis queue

    if (vkCreateBuffer(m_device.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device.getDevice(), buffer, &memRequirements);
    // printf("Info: Buffer alignment: %d\n", memRequirements.alignment); // 256 on Nvidia GeForce 940MX

    // Allocate the memory
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties); // find apropriate memory

    if (vkAllocateMemory(m_device.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to allocate buffer memory!");
    }

    // Bind the buffer to the memory
    // The offset has to be a multiple of memRequirements.alignment
    vkBindBufferMemory(m_device.getDevice(), buffer, bufferMemory, 0 /*offset*/);
}

/* Copies the contents of a source buffer to a destination buffer on the GPU. */
void VulkanMemory::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // Put copy cmd
    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);

}

/* Copies data to buffer. */
void VulkanMemory::copyDataToBuffer(VkBuffer buffer, VkDeviceMemory memory, const void *data, size_t size) {
    void *bufferData;
    vkMapMemory(m_device.getDevice(), memory, 0, size, 0, &bufferData);
    memcpy(bufferData, data, (size_t) size);
    vkUnmapMemory(m_device.getDevice(), memory);
}

void VulkanMemory::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

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

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

/* Creates a new command buffer for single time use and begins recording it. 
	TOBE moved
	*/
VkCommandBuffer VulkanMemory::beginSingleTimeCommands() {
    // Should be in its own command pool which has the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT enabled during creation
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1; // we only need one

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device.getDevice(), &allocInfo, &commandBuffer);

    // Begin the command buffer
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // will be rerecorded after submit

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

/* Ends the command buffer, submits it to the queue, waits for it to finish and deletes the buffer.
	TOBE moved
	*/
void VulkanMemory::endSingleTimeCommands(VkCommandBuffer &commandBuffer) {
    // Finish command buffer
    vkEndCommandBuffer(commandBuffer);

    // Submit this cmdbuffer to the queue
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    // Wait for it to finish
    vkQueueWaitIdle(m_device.getGraphicsQueue());

    // The cmdbuffer is no longer needed
    vkFreeCommandBuffers(m_device.getDevice(), m_commandPool, 1, &commandBuffer);
}

/* Finds apropriate memory type if the physical device */
uint32_t VulkanMemory::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_device.getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        // check if the memory support the required features
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("VULKAN: failed to find suitable memory type!");
}

const VulkanUniformBuffer
VulkanMemory::createUniformBuffer(uint32_t elementSize, VkBufferCreateFlags flags, uint32_t count, bool aligned) {
    VkDeviceSize uboSize = (long) elementSize * count;
    VkDeviceSize alignment = 0;
    if (aligned) { // Align the data
        alignment = m_device.getProperties().limits.minUniformBufferOffsetAlignment;
        uboSize = (elementSize < alignment) ? alignment : (elementSize + (elementSize % alignment));
    }

    VkDeviceSize bufferSize = uboSize * count;


    VkBuffer buffer;
    VkDeviceMemory memory;
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, flags,
                 buffer, memory);

    return VulkanUniformBuffer{buffer, memory, uboSize, alignment};
}

/* Creates a buffer containing 'size' bytes from 'data'.
	The buffer is a transfer_dst and device_local.
	The data is transmitted using a staging buffer. 
	*/
const VulkanBuffer VulkanMemory::createInputBuffer(VkDeviceSize size, void *data, VkBufferUsageFlags flags) {
    // Staging buffer to contain data for transfer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    // Creates buffer with usage=transfer_src, host visible and coherent meaning the cpu has access to the memory and changes are immediately known to the driver which will transfer the memmory before the next vkQueueSubmit
    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferMemory);

    copyDataToBuffer(stagingBuffer, stagingBufferMemory, data, size);

    // Vertex buffer
    VkBuffer inputBuffer;
    VkDeviceMemory inputBufferMemory;
    // Create vertex buffer, usage=transfer_dst | vertexbuffer, it is device local
    createBuffer(size, flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, inputBuffer,
                 inputBufferMemory);

    // Create copy operation to move data from staging into vertex buffer
    copyBuffer(stagingBuffer, inputBuffer, size);

    // The staging buffer is no longer necessary so we can destroy it
    vkDestroyBuffer(m_device.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_device.getDevice(), stagingBufferMemory, nullptr);

    return VulkanBuffer{.buffer=inputBuffer, .memory=inputBufferMemory};
}