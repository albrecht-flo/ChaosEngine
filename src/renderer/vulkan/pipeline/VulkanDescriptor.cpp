#include "VulkanDescriptor.h"

#include <stdexcept>

DescriptorSetLayout VulkanDescriptor::createDescriptorSetLayout(
        VulkanDevice &device,
        std::vector<VkDescriptorSetLayoutBinding> descriptorBindings) {

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(descriptorBindings.size());
    layoutInfo.pBindings = descriptorBindings.data();

    VkDescriptorSetLayout layout;
    if (vkCreateDescriptorSetLayout(device.vk(), &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create descriptor set layout!");
    }

    return DescriptorSetLayout{layout, descriptorBindings};
}

VkDescriptorPool VulkanDescriptor::createPool(VulkanDevice &device,
                                              std::vector<VkDescriptorPoolSize> poolSizes) {
    uint32_t maxSets = 0;
    for (auto &poolSize : poolSizes)
        maxSets += poolSize.descriptorCount;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSets; // max number of sets to be allocated

    VkDescriptorPool descriptorPool;
    if (vkCreateDescriptorPool(device.vk(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create descriptor pool!");
    }

#ifdef M_DEBUG
    printf("Created pool(%x) with maxSets = %d, types: ", descriptorPool, maxSets);
    for(auto& t : poolSizes)
        printf("(%d, %d), ", t.type, t.descriptorCount);
    printf("\n");
#endif // DEBUG

    return descriptorPool;
}

VkDescriptorSet VulkanDescriptor::allocateDescriptorSet(VulkanDevice &device,
                                                        DescriptorSetLayout layout, VkDescriptorPool descriptorPool) {
    // Create the descriptor sets
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool; // the pool from which to create them
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout.vDescriptorSetLayout; // the layouts

    VkDescriptorSet descriptorSet;
    if (vkAllocateDescriptorSets(device.vk(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to allocate descriptor set!");
    }
#ifdef M_DEBUG
    printf("Allocated new descriptor set(%x) from %x\n", descriptorSet, descriptorPool);
#endif // DEBUG

    return descriptorSet;
}

void VulkanDescriptor::writeDescriptorSet(VulkanDevice &device, VkDescriptorSet descriptorSet,
                                          std::vector<DescriptorBufferInfo> bufferInfos,
                                          std::vector<DescriptorImageInfo> imageInfos) {

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.resize(bufferInfos.size() + imageInfos.size());

    unsigned int i = 0;
    for (DescriptorBufferInfo &info : bufferInfos) {
        descriptorWrites[i] = {};
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = descriptorSet;
        descriptorWrites[i].dstBinding = info.binding;
        descriptorWrites[i].dstArrayElement = info.arrayElement;
        descriptorWrites[i].descriptorType = info.type;
        descriptorWrites[i].descriptorCount = info.count;
        descriptorWrites[i].pBufferInfo = &info.descriptorInfo;
        descriptorWrites[i].pImageInfo = nullptr;
        descriptorWrites[i].pTexelBufferView = nullptr;

        i++;
    }

    for (DescriptorImageInfo &info : imageInfos) {
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = descriptorSet;
        descriptorWrites[i].dstBinding = info.binding;
        descriptorWrites[i].dstArrayElement = info.arrayElement;
        descriptorWrites[i].descriptorType = info.type;
        descriptorWrites[i].descriptorCount = info.count;
        descriptorWrites[i].pImageInfo = &info.descriptorInfo;

        i++;
    }

    vkUpdateDescriptorSets(device.vk(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
                           0, nullptr);
}