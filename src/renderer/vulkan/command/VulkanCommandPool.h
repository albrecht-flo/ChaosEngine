#pragma once

#include <vulkan/vulkan.h>

class VulkanDevice;

class VulkanCommandPool {
private:
    VulkanCommandPool(const VulkanDevice &device, VkCommandPool commandPool);

public:
    ~VulkanCommandPool();

    VulkanCommandPool(const VulkanCommandPool &o) = delete;

    VulkanCommandPool &operator=(const VulkanCommandPool &o) = delete;

    VulkanCommandPool(VulkanCommandPool &&o) noexcept;

    VulkanCommandPool &operator=(VulkanCommandPool &&o) = delete;

    static VulkanCommandPool Create(const VulkanDevice &device);

    [[nodiscard]] inline VkCommandPool vk() const { return commandPool; }

private:
    void destroy();

private:
    const VulkanDevice &device;
    VkCommandPool commandPool;

};

