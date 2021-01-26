#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <vector>

#include "src/renderer/vulkan/context/VulkanDevice.h"

struct DescriptorSetLayout {
    VkDescriptorSetLayout vDescriptorSetLayout;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct DescriptorBufferInfo {
    VkDescriptorBufferInfo descriptorInfo;
    uint32_t binding;
    uint32_t arrayElement;
    VkDescriptorType type;
    uint32_t count;
};

struct DescriptorImageInfo {
    VkDescriptorImageInfo descriptorInfo;
    uint32_t binding;
    uint32_t arrayElement;
    VkDescriptorType type;
    uint32_t count;
};

class VulkanDescriptor {
public:
    static DescriptorSetLayout createDescriptorSetLayout(
            VulkanDevice &device,
            std::vector<VkDescriptorSetLayoutBinding> descriptorBindings);

    static VkDescriptorPool createPool(VulkanDevice &device,
                                       std::vector<VkDescriptorPoolSize> poolSizes);

    static VkDescriptorSet allocateDescriptorSet(VulkanDevice &device,
                                                 DescriptorSetLayout layout, VkDescriptorPool descriptorPool);

    static void writeDescriptorSet(VulkanDevice &device, VkDescriptorSet descriptorSet,
                                   std::vector<DescriptorBufferInfo> bufferInfos,
                                   std::vector<DescriptorImageInfo> imageInfos = {}
    );
};