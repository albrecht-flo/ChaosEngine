#include "VulkanDescriptorPool.h"
#include "VulkanDescriptorSetLayout.h"

VulkanDescriptorSet VulkanDescriptorPool::allocate(const VulkanDescriptorSetLayout &layout) const {

    // Create the descriptor sets
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    auto vDescSetLayout = layout.vk();
    allocInfo.pSetLayouts = &vDescSetLayout;

    VkDescriptorSet descriptorSet{};
    if (vkAllocateDescriptorSets(device.vk(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to allocate descriptor set!");
    }

    return VulkanDescriptorSet{device, descriptorSet};
}