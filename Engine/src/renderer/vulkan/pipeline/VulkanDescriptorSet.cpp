#include "VulkanDescriptorSet.h"

#include "VulkanDescriptorSetLayout.h"

#include <stdexcept>

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
    for (const auto &desc: descriptorWrites) {
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