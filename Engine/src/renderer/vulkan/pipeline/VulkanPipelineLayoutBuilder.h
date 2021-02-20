#pragma once

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanPipelineLayout.h"

class VulkanDescriptorSetLayout;

/**
 * This class controls the creation of a pipeline layout.
 */
class VulkanPipelineLayoutBuilder {
public:
    explicit VulkanPipelineLayoutBuilder(const VulkanDevice &device)
            : device(device) {}

    ~VulkanPipelineLayoutBuilder() = default;

    VulkanPipelineLayoutBuilder &addPushConstant(unsigned int size, unsigned int offset, ShaderStage stage);

    VulkanPipelineLayout build();


    VulkanPipelineLayoutBuilder &addDescriptorSet(const VulkanDescriptorSetLayout &layout);

private:
    const VulkanDevice &device;
    std::vector<struct VkDescriptorSetLayout_T *> layouts;
    std::vector<VkPushConstantRange> pushConstants;
};

