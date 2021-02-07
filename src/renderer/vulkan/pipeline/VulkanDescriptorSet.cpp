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
void
VulkanDescriptorSetOperation::writeBuffer(VkDescriptorSet descriptorSet, uint32_t binding, VkBuffer buffer,
                                          uint64_t bufferOffset,
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

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.push_back(writeInfo);
    vkUpdateDescriptorSets(device.vk(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
                           0, nullptr);
}

VulkanDescriptorSetOperation &
VulkanDescriptorSetOperation::writeBuffer(uint32_t binding, VkBuffer buffer, uint64_t bufferOffset,
                                          uint64_t bufferRange, uint32_t arrayElement,
                                          uint32_t descriptorCount) {
    VkDescriptorBufferInfo bufferInfo{
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
    writeInfo.pBufferInfo = nullptr; // To be set at commmit

    descriptorWrites.emplace_back(
            DescriptorInfo{.type = DescriptorInfoType::Buffer, .descriptorWrite=writeInfo, .bufferInfo=bufferInfo});
    confirmed = false;
    return *this;
}


VulkanDescriptorSetOperation &
VulkanDescriptorSetOperation::writeImageSampler(uint32_t binding, VkSampler sampler,
                                                VkImageView imageView,
                                                VkImageLayout imageLayout,
                                                uint32_t arrayElement, uint32_t descriptorCount) {

    VkDescriptorImageInfo imageInfo{
            .sampler = sampler,
            .imageView = imageView,
            .imageLayout = imageLayout,
    };

    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = descriptorSet;
    writeInfo.dstBinding = binding;
    writeInfo.dstArrayElement = arrayElement;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfo.descriptorCount = descriptorCount;
    writeInfo.pImageInfo = nullptr; // to be set a commit

    descriptorWrites.emplace_back(
            DescriptorInfo{.type = DescriptorInfoType::Image, .descriptorWrite=writeInfo, .imageInfo=imageInfo});
    confirmed = false;
    return *this;
}

void VulkanDescriptorSetOperation::commit() {
    std::vector<VkWriteDescriptorSet> writes;
    writes.reserve(descriptorWrites.size());
    for (const auto& desc : descriptorWrites) {
        writes.emplace_back(desc.descriptorWrite);
        if (desc.type == DescriptorInfoType::Buffer) {
            writes.back().pBufferInfo = &desc.bufferInfo;
        } else if (desc.type == DescriptorInfoType::Image) {
            writes.back().pImageInfo = &desc.imageInfo;
        } else { assert(("Missing branch for type in commit!", false)); }
    }

    vkUpdateDescriptorSets(device.vk(), static_cast<uint32_t>(writes.size()), writes.data(),
                           0, nullptr);
    confirmed = true;
}