#include "VulkanCommandPool.h"

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"

#include <stdexcept>
#include <iostream>
#include <cassert>

static VkCommandPool createCommandPool(const VulkanDevice &device, uint32_t queueFamilyIndex) {
    VkCommandPoolCreateFlagBits flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (queueFamilyIndex == device.getTransferQueueFamilyIndex()) {
        // Command buffers from the transfer pool are going to be short lived
        flags = static_cast<VkCommandPoolCreateFlagBits>(flags | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    }
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = flags; // We want the buffers to be able to reset them
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool{};
    if (vkCreateCommandPool(device.vk(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create graphics command pool!");
    }
    return commandPool;
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanCommandPool VulkanCommandPool::Create(const VulkanDevice &device, uint32_t queueFamilyIndex, VkQueue queue) {
    auto commandPool = createCommandPool(device, queueFamilyIndex);
    return VulkanCommandPool(device, commandPool, queue);
}

VulkanCommandPool::VulkanCommandPool(const VulkanDevice &device, VkCommandPool commandPool, VkQueue queue)
        : device(device), commandPool(commandPool), queue(queue) {
    assert("Queue of CommandPool must not be NULL!" && queue != nullptr);
}

VulkanCommandPool::VulkanCommandPool(VulkanCommandPool &&o) noexcept
        : device(o.device), commandPool(std::exchange(o.commandPool, nullptr)),
          queue(std::exchange(o.queue, nullptr)) {}

VulkanCommandPool::~VulkanCommandPool() {
    destroy();
}

void VulkanCommandPool::destroy() {
    if (commandPool != nullptr)
        vkDestroyCommandPool(device.vk(), commandPool, nullptr);
}

void VulkanCommandPool::runInSingeTimeCommandBuffer(std::function<void(VkCommandBuffer)> &&func) const {
    // Should be in its own command pool which has the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT enabled during creation
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1; // we only need one

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device.vk(), &allocInfo, &commandBuffer);

    // Begin the command buffer
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // will be rerecorded after submit

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // Run caller function
    func(commandBuffer);

    // Finish command buffer
    vkEndCommandBuffer(commandBuffer);

    // Submit this command buffer to the queue
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    // Wait for it to finish
    vkQueueWaitIdle(queue);

    // This command buffer is no longer needed
    vkFreeCommandBuffers(device.vk(), commandPool, 1, &commandBuffer);
}
