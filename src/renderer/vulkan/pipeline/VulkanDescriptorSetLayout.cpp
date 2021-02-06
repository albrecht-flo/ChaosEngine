#include "VulkanDescriptorSetLayout.h"

#include "src/renderer/vulkan/context/VulkanDevice.h"

#include <stdexcept>

using namespace VulkanPipelineUtility;

// ----------------------- VulkanDescriptorSetLayoutBuilder Class Members -------------------------------------------------

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

// ----------------------- VulkanDescriptorSetLayout Class Members -----------------------------------------------------

void VulkanDescriptorSetLayout::destroy() {
    if (layout != nullptr) vkDestroyDescriptorSetLayout(device.vk(), layout, nullptr);
}

// ------------------ VulkanPipelineLayoutBuilder Class Members --------------------------------------------------------

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

// ----------------------- VulkanPipelineLayout Class Members ----------------------------------------------------------

VulkanPipelineLayout &VulkanPipelineLayout::operator=(VulkanPipelineLayout &&o) noexcept {
    if (this == &o)
        return *this;
    destroy();
    layout = std::exchange(o.layout, nullptr);
    return *this;
}


void VulkanPipelineLayout::destroy() {
    if (layout != nullptr) vkDestroyPipelineLayout(device.vk(), layout, nullptr);
}
