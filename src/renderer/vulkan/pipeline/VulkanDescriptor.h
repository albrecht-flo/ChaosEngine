#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanDescriptorSetLayout.h"

struct DescriptorSetLayout {
    VkDescriptorSetLayout vDescriptorSetLayout;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct PipelineLayout {
    std::vector<DescriptorSetLayout> layouts;
    std::vector<VkPushConstantRange> pushConstants;
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

// TODO: Create Builder
//  https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
//  https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets
class VulkanDescriptor {
public:
    static DescriptorSetLayout createDescriptorSetLayout(
            const VulkanDevice &device,
            std::vector<VkDescriptorSetLayoutBinding> descriptorBindings);

    static VkDescriptorPool createPool(const VulkanDevice &device,
                                       std::vector<VkDescriptorPoolSize> poolSizes);

    static VkDescriptorSet allocateDescriptorSet(const VulkanDevice &device,
                                                 const VulkanDescriptorSetLayout &layout, VkDescriptorPool descriptorPool);

    static void writeDescriptorSet(const VulkanDevice &device, VkDescriptorSet descriptorSet,
                                   std::vector<DescriptorBufferInfo> bufferInfos,
                                   std::vector<DescriptorImageInfo> imageInfos = {}
    );
};