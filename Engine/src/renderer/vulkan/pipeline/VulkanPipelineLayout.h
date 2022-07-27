#pragma once

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"

#include <utility>

/**
 * This class defines the dynamic layout of a pipeline combining, push constants and descriptor set layouts.
 */
class VulkanPipelineLayout {
    friend class VulkanPipelineLayoutBuilder;

private:
    VulkanPipelineLayout(const VulkanDevice &device, VkPipelineLayout layout)
            : device(device), layout(layout) {}

    void destroy();

public:
    ~VulkanPipelineLayout() { destroy(); }

    VulkanPipelineLayout(const VulkanPipelineLayout &o) = delete;

    VulkanPipelineLayout &operator=(const VulkanPipelineLayout &o) = delete;

    VulkanPipelineLayout(VulkanPipelineLayout &&o) noexcept
            : device(o.device), layout(std::exchange(o.layout, nullptr)) {}

    VulkanPipelineLayout &operator=(VulkanPipelineLayout &&o) noexcept;

    [[nodiscard]] inline VkPipelineLayout vk() const { return layout; }

private:
    const VulkanDevice &device;
    VkPipelineLayout layout;
};


