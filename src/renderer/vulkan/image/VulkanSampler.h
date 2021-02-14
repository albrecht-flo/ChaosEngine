#pragma once

#include "src/renderer/vulkan/context/VulkanDevice.h"

class VulkanSampler {
private:
    VulkanSampler(const VulkanDevice &device, VkSampler sampler) : device(device), sampler(sampler) {}

    void destroy();

public:
    [[deprecated]] VulkanSampler(const VulkanDevice &device) : device(device), sampler(nullptr) {}

    ~VulkanSampler() { destroy(); }

    VulkanSampler(const VulkanSampler &o) = delete;

    VulkanSampler &operator=(const VulkanSampler &o) = delete;

    VulkanSampler(VulkanSampler &&o) noexcept: device(o.device), sampler(std::exchange(o.sampler, nullptr)) {}

    VulkanSampler &operator=(VulkanSampler &&o) noexcept {
        if (this == &o)
            return *this;
        destroy();
        sampler = std::exchange(o.sampler, nullptr);
        return *this;
    }

    static VulkanSampler create(const VulkanDevice &device, VkFilter filter = VK_FILTER_LINEAR);

    [[nodiscard]] inline VkSampler vk() const { return sampler; }

private:
    const VulkanDevice &device;
    VkSampler sampler;
};

