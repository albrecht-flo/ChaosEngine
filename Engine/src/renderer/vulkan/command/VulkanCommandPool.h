#pragma once

#include <vulkan/vulkan.h>

#include <functional>

class VulkanDevice;

class VulkanCommandPool {
private:
    VulkanCommandPool(const VulkanDevice &device, VkCommandPool commandPool, VkQueue queue);

public:
    ~VulkanCommandPool();

    VulkanCommandPool(const VulkanCommandPool &o) = delete;

    VulkanCommandPool &operator=(const VulkanCommandPool &o) = delete;

    VulkanCommandPool(VulkanCommandPool &&o) noexcept;

    VulkanCommandPool &operator=(VulkanCommandPool &&o) = delete;

    static VulkanCommandPool Create(const VulkanDevice &device, uint32_t queueFamilyIndex, VkQueue queue);

    [[nodiscard]] inline VkCommandPool vk() const { return commandPool; }

    void runInSingeTimeCommandBuffer(std::function<void(VkCommandBuffer)> &&func) const;

private:
    void destroy();

private:
    const VulkanDevice &device;
    VkCommandPool commandPool;
    VkQueue queue;

};

