#pragma once

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanDescriptorSet.h"

class VulkanDescriptorPool {
    friend class VulkanDescriptorPoolBuilder;

private:
    VulkanDescriptorPool(const VulkanDevice &device, VkDescriptorPool descriptorPool)
            : device(device), descriptorPool(descriptorPool) {}

    /// Destroys this descriptor pool and all allocated sets from this pool
    void destroy() {
        if (descriptorPool != nullptr)
            vkDestroyDescriptorPool(device.vk(), descriptorPool, nullptr);
    }

public:
    ~VulkanDescriptorPool() { destroy(); }

    VulkanDescriptorPool(const VulkanDescriptorPool &o) = delete;

    VulkanDescriptorPool &operator=(const VulkanDescriptorPool &o) = delete;

    VulkanDescriptorPool(VulkanDescriptorPool &&o) noexcept
            : device(o.device), descriptorPool(std::exchange(o.descriptorPool, nullptr)) {}

    VulkanDescriptorPool &operator=(VulkanDescriptorPool &&o) noexcept {
        if (this == &o)
            return *this;
        destroy();
        descriptorPool = std::exchange(o.descriptorPool, nullptr);
        return *this;
    }

    [[nodiscard]] VulkanDescriptorSet allocate(const VulkanDescriptorSetLayout &layout) const;

    [[nodiscard]] inline VkDescriptorPool vk() const { return descriptorPool; }

private:
    const VulkanDevice &device;
    VkDescriptorPool descriptorPool;
};

