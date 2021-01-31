#include "VulkanCommandBuffer.h"

#include <vulkan/vulkan.h>

#include <stdexcept>

static VkCommandBuffer createCommandBuffer(const VulkanDevice &device, VkCommandPool commandPool,
                                           VkCommandBufferLevel level) {
    // Creates the command buffers for each frame
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool; // the pool to allocate from
    allocInfo.level = level; // primary -> can be submited directly; secondary -> can be called from primary
    allocInfo.commandBufferCount = 1; // the number of buffers

    VkCommandBuffer commandBuffer{};
    if (vkAllocateCommandBuffers(device.vk(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to allocate command buffers!");
    }
    return commandBuffer;
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanCommandBuffer
VulkanCommandBuffer::Create(const VulkanDevice &device, const VulkanCommandPool &commandPool,
                            VkCommandBufferLevel level) {
    auto commandBuffer = createCommandBuffer(device, commandPool.vk(), level);

    return VulkanCommandBuffer{device, commandPool, commandBuffer};

}

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanDevice &device, const VulkanCommandPool &commandPool,
                                         VkCommandBufferLevel level) :
        device(device), commandPool(commandPool), buffer(createCommandBuffer(device, commandPool.vk(), level)) {}

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanDevice &device, const VulkanCommandPool &commandPool,
                                         VkCommandBuffer buffer)
        : device(device), commandPool(commandPool), buffer(buffer) {}

VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer &&o) noexcept
        : device(o.device), commandPool(o.commandPool), buffer(std::exchange(o.buffer, nullptr)) {}


VulkanCommandBuffer::~VulkanCommandBuffer() {
    destroy();
}

/* Begin recording the command buffer.
	If allocated from a command pool with reset flag implicitly resets the buffer.
*/
void VulkanCommandBuffer::begin(VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flags; // can be resubmited while beeing executed
    beginInfo.pInheritanceInfo = nullptr; // only relevant for secondary cmdbuffers

    if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to begin recording command buffer!");
    }
}

// Finish recording the command buffer
void VulkanCommandBuffer::end() {
    if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to record command buffer!");
    }
}

void VulkanCommandBuffer::destroy() {
    vkFreeCommandBuffers(device.vk(), commandPool.vk(), 1, &buffer);
}
