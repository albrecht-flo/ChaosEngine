#pragma once

#include "src/renderer/vulkan/context/VulkanDevice.h"

#include <vector>
#include <cassert>

class VulkanDescriptorSetLayout;


class VulkanDescriptorSetLayout {
    friend class VulkanDescriptorSetLayoutBuilder;

private:
    VulkanDescriptorSetLayout(const VulkanDevice &device, VkDescriptorSetLayout layout,
                              std::vector<VkDescriptorSetLayoutBinding> descriptorBindings)
            : device(device), layout(layout), descriptorBindings(std::move(descriptorBindings)) {}

    void destroy();

public:
    ~VulkanDescriptorSetLayout() { destroy(); }

    VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &o) = delete;

    VulkanDescriptorSetLayout &operator=(const VulkanDescriptorSetLayout &o) = delete;

    VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&o) noexcept
            : device(o.device), layout(std::exchange(o.layout, nullptr)),
              descriptorBindings(std::move(o.descriptorBindings)) {}

    VulkanDescriptorSetLayout &operator=(VulkanDescriptorSetLayout &&o) = delete;

    [[nodiscard]] inline VkDescriptorSetLayout vk() const { return layout; }

    inline VkDescriptorSetLayoutBinding getBinding(uint32_t binding) {
        assert(binding < descriptorBindings.size());
        return descriptorBindings[binding];
    }

private:
    const VulkanDevice &device;
    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;
};


