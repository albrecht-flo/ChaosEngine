#include "VulkanDescriptorSet.h"

#include "src/renderer/vulkan/image/VulkanImageView.h"
#include "VulkanDescriptorSetLayout.h"

#include <stdexcept>

// ----------------------- VulkanDescriptorPoolBuilder  Class Members --------------------------------------------------

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

// --------------------------- VulkanDescriptorPool  Class Members -----------------------------------------------------

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

// --------------------------- VulkanDescriptorSet Class Members -------------------------------------------------------
VulkanDescriptorSet::VulkanDescriptorSetOperation &
VulkanDescriptorSet::VulkanDescriptorSetOperation::writeBuffer(uint32_t binding, VkBuffer buffer, uint64_t bufferOffset,
                                                               uint64_t bufferRange, uint32_t arrayElement,
                                                               uint32_t descriptorCount) {
    VkDescriptorBufferInfo info{
            .buffer = buffer,
            .offset = bufferOffset,
            .range = bufferRange,
    };

    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = descriptorSet;
    writeInfo.dstBinding = binding;
    writeInfo.dstArrayElement = arrayElement;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeInfo.descriptorCount = descriptorCount;
    writeInfo.pBufferInfo = &info;
    writeInfo.pImageInfo = nullptr;
    writeInfo.pTexelBufferView = nullptr;

    descriptorWrites.emplace_back(writeInfo);
    confirmed = true;
    return *this;
}

VulkanDescriptorSet::VulkanDescriptorSetOperation &
VulkanDescriptorSet::VulkanDescriptorSetOperation::writeImageSampler(uint32_t binding, VkSampler sampler,
                                                                     const VulkanImageView &imageView,
                                                                     VkImageLayout imageLayout,
                                                                     uint32_t arrayElement, uint32_t descriptorCount) {

    VkDescriptorImageInfo info{
            .sampler = sampler,
            .imageView = imageView.vk(),
            .imageLayout = imageLayout,
    };

    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = descriptorSet;
    writeInfo.dstBinding = binding;
    writeInfo.dstArrayElement = arrayElement;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfo.descriptorCount = descriptorCount;
    writeInfo.pImageInfo = &info;

    descriptorWrites.emplace_back(writeInfo);
    confirmed = true;
    return *this;
}
