#include "VulkanDescriptorSetLayoutBuilder.h"

#include <stdexcept>

using namespace VulkanPipelineUtility;

VulkanDescriptorSetLayoutBuilder &
VulkanDescriptorSetLayoutBuilder::addBinding(uint32_t binding, DescriptorType type, ShaderStage stage, uint32_t size) {
    assert(("Bindings must have different binding indices!",
            std::find_if(descriptorBindings.begin(), descriptorBindings.end(),
                         [&](VkDescriptorSetLayoutBinding b) { return b.binding == binding; }) ==
            descriptorBindings.end()));
    assert(("Descriptor count needs to be > 0", size > 0));

    descriptorBindings.emplace_back(VkDescriptorSetLayoutBinding{
            .binding = binding,
            .descriptorType = getVkDescriptorType(type),
            .descriptorCount = size,
            .stageFlags = getVkShaderStage(stage),
            .pImmutableSamplers = nullptr
    });

    return *this;
}

VulkanDescriptorSetLayout VulkanDescriptorSetLayoutBuilder::build() {
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(descriptorBindings.size());
    layoutInfo.pBindings = descriptorBindings.data();

    VkDescriptorSetLayout layout;
    if (vkCreateDescriptorSetLayout(device.vk(), &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create descriptor set layout!");
    }

    return VulkanDescriptorSetLayout{device, layout, descriptorBindings};
}
