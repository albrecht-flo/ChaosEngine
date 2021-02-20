#include "VulkanCommandPool.h"

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"

#include <stdexcept>
#include <iostream>

/// Creates command pool to contain command buffers
static VkCommandPool createCommandPool(const VulkanDevice &device) {
    QueueFamilyIndices queueFamilyIndices = device.findQueueFamilies();

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // We want the buffers to be able to reset them
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkCommandPool commandPool{};
    if (vkCreateCommandPool(device.vk(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create graphics command pool!");
    }
    return commandPool;
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanCommandPool VulkanCommandPool::Create(const VulkanDevice &device) {
    auto commandPool = createCommandPool(device);
    return VulkanCommandPool(device, commandPool);
}

VulkanCommandPool::VulkanCommandPool(const VulkanDevice &device, VkCommandPool commandPool)
        : device(device), commandPool(commandPool) {}

VulkanCommandPool::VulkanCommandPool(VulkanCommandPool &&o) noexcept
        : device(o.device), commandPool(std::exchange(o.commandPool, nullptr)) {}

VulkanCommandPool::~VulkanCommandPool() {
    destroy();
}

// Destroy the command pool
void VulkanCommandPool::destroy() {
    if (commandPool != nullptr)
        vkDestroyCommandPool(device.vk(), commandPool, nullptr);
}
