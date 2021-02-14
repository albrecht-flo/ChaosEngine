#pragma once

#include "src/renderer/vulkan/context/VulkanDevice.h"

#include <iostream>
#include <cassert>

class VulkanImageView;

class VulkanDescriptorSetLayout;

/**
 * This class manages writing to a descriptor set in a command like fashion.
 * @note Must be committed before it gets destroyed
 */
class VulkanDescriptorSetOperation {
public:
    VulkanDescriptorSetOperation(const VulkanDevice &device, VkDescriptorSet descriptorSet)
            : device(device), descriptorSet(descriptorSet) {}

    ~VulkanDescriptorSetOperation() {
        if (!confirmed) {
            std::cerr << "[WARNING] Destroying an uncommitted DescriptorSet operation" << std::endl;
        }
    }

    VulkanDescriptorSetOperation &
    writeBuffer(uint32_t binding, VkBuffer buffer, uint64_t bufferOffset = 0, uint64_t bufferRange = VK_WHOLE_SIZE,
                uint32_t arrayElement = 0, uint32_t descriptorCount = 1);

    VulkanDescriptorSetOperation &
    writeImageSampler(uint32_t binding, VkSampler sampler, VkImageView imageView,
                      VkImageLayout imageLayout, uint32_t arrayElement = 0, uint32_t descriptorCount = 1);

    void commit();

private:
    enum class DescriptorInfoType {
        Buffer, Image
    };
    struct DescriptorInfo {
        DescriptorInfoType type;
        VkWriteDescriptorSet descriptorWrite;
        union {
            VkDescriptorBufferInfo bufferInfo;
            VkDescriptorImageInfo imageInfo;
        };
    };
private:
    const VulkanDevice &device;
    VkDescriptorSet descriptorSet;
    std::vector<DescriptorInfo> descriptorWrites;
    bool confirmed = false;

};

/**
 * Descriptor-Sets represent a collection of GPU resources that can be bound to a pipeline.
 * They are allocated from a VulkanDescriptorPool.
 *
 * @ref VulkanDescriptorPool::allocate
 */
class VulkanDescriptorSet {
    friend class VulkanDescriptorPool;

private:
    VulkanDescriptorSet(const VulkanDevice &device, VkDescriptorSet descriptorSet)
            : device(device), descriptorSet(descriptorSet) {}

public:
    ~VulkanDescriptorSet() = default;

    VulkanDescriptorSetOperation startWriting() {
        return VulkanDescriptorSetOperation(device, descriptorSet);
    }

    [[nodiscard]] inline VkDescriptorSet vk() const { return descriptorSet; }

private:
    const VulkanDevice &device;
    VkDescriptorSet descriptorSet;
};

