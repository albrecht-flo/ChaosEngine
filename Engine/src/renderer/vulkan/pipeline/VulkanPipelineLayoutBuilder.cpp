#include "VulkanPipelineLayoutBuilder.h"

#include <stdexcept>

using namespace VulkanPipelineUtility;

VulkanPipelineLayoutBuilder &
VulkanPipelineLayoutBuilder::addPushConstant(uint32_t size, uint32_t offset, ShaderStage stage) {
    auto pushConstant = VkPushConstantRange{
            .stageFlags = getVkShaderStage(stage),
            .offset = offset,
            .size = size,
    };

    pushConstants.emplace_back(pushConstant);
    return *this;
}

VulkanPipelineLayoutBuilder &
VulkanPipelineLayoutBuilder::addDescriptorSet(const VulkanDescriptorSetLayout &layout) {
    layouts.emplace_back(layout.vk());
    return *this;
}

VulkanPipelineLayout VulkanPipelineLayoutBuilder::build() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

    VkPipelineLayout pipelineLayout{};
    if (vkCreatePipelineLayout(device.vk(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create pipeline layout!");
    }

    return VulkanPipelineLayout{device, pipelineLayout};
}
