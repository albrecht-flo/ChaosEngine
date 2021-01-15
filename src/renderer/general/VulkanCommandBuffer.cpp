#include "VulkanCommandBuffer.h"

#include <stdexcept>

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice &device, VkCommandPool commandPool, VkCommandBufferLevel level) :
        m_device(device), m_commandPool(commandPool) {

    // Creates the command buffers for each frame
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool; // the pool to allocate from
    allocInfo.level = level; // primary -> can be submited directly; secondary -> can be called from primary
    allocInfo.commandBufferCount = 1; // the number of buffers

    if (vkAllocateCommandBuffers(m_device.getDevice(), &allocInfo, &m_buffer) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to allocate command buffers!");
    }
}

/* Begin recording the command buffer.
	If allocated from a command pool with reset flag implicitly resets the buffer.
*/
void VulkanCommandBuffer::begin(VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flags; // can be resubmited while beeing executed
    beginInfo.pInheritanceInfo = nullptr; // only relevant for secondary cmdbuffers

    if (vkBeginCommandBuffer(m_buffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to begin recording command buffer!");
    }
}

// Finish recording the command buffer
void VulkanCommandBuffer::end() {
    if (vkEndCommandBuffer(m_buffer) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to record command buffer!");
    }
}

void VulkanCommandBuffer::destroy() {
    vkFreeCommandBuffers(m_device.getDevice(), m_commandPool, 1, &m_buffer);
}

VulkanCommandBuffer::~VulkanCommandBuffer() {

}