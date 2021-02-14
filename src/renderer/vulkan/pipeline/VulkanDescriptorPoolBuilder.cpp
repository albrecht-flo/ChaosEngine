#include "VulkanDescriptorPoolBuilder.h"

VulkanDescriptorPool VulkanDescriptorPoolBuilder::build() const {
    assert(("MaxSets must be more than 0", maxSets > 0));

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorSizes.size());
    poolInfo.pPoolSizes = descriptorSizes.data(); // max # per descriptor type
    poolInfo.maxSets = maxSets; // max number of sets to be allocated

    VkDescriptorPool descriptorPool{};
    if (vkCreateDescriptorPool(device.vk(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create descriptor pool!");
    }

    return VulkanDescriptorPool{device, descriptorPool};
}