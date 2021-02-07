#pragma once

#include <vulkan/vulkan.h>
#include "src/renderer/vulkan/context/VulkanDevice.h"

#include <iostream>
#include <cassert>

class VulkanImageView;

class VulkanDescriptorSetLayout;

class VulkanDescriptorPool;

class VulkanDescriptorSetOperation {
public:
    VulkanDescriptorSetOperation(const VulkanDevice &device, VkDescriptorSet descriptorSet)
            : device(device), descriptorSet(descriptorSet) {}

    ~VulkanDescriptorSetOperation() {
        if (!confirmed) {
            std::cerr << "[WARNING] Destroying an uncommitted DescriptorSet operation" << std::endl;
        }
    }

    void
    writeBuffer(VkDescriptorSet descriptorSet, uint32_t binding, VkBuffer buffer, uint64_t bufferOffset = 0,
                uint64_t bufferRange = VK_WHOLE_SIZE,
                uint32_t arrayElement = 0, uint32_t descriptorCount = 1);


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


class VulkanDescriptorSet {
    friend VulkanDescriptorPool;
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

class VulkanDescriptorPoolBuilder {
public:
    explicit VulkanDescriptorPoolBuilder(const VulkanDevice &device)
            : device(device), maxSets(0) {}

    VulkanDescriptorPoolBuilder &addDescriptor(VkDescriptorType type, uint32_t count) {
        descriptorSizes.emplace_back(VkDescriptorPoolSize{
                .type= type,
                .descriptorCount = count,
        });
        return *this;
    }

    VulkanDescriptorPoolBuilder &setMaxSets(uint32_t pMaxSets) {
        maxSets = pMaxSets;
        return *this;
    }

    [[nodiscard]] VulkanDescriptorPool build() const;

private:
    const VulkanDevice &device;
    uint32_t maxSets;
    std::vector<VkDescriptorPoolSize> descriptorSizes;
};

class VulkanDescriptorPool {
    friend VulkanDescriptorPoolBuilder;
private:
    VulkanDescriptorPool(const VulkanDevice &device, VkDescriptorPool descriptorPool)
            : device(device), descriptorPool(descriptorPool) {}

    /// Destroys this descriptor pool and all allocated sets from this pool
    void destroy() {
        if (descriptorPool != nullptr)
            vkDestroyDescriptorPool(device.vk(), descriptorPool, nullptr);
    }

public:
    ~VulkanDescriptorPool() { destroy(); }

    VulkanDescriptorPool(const VulkanDescriptorPool &o) = delete;

    VulkanDescriptorPool &operator=(const VulkanDescriptorPool &o) = delete;

    VulkanDescriptorPool(VulkanDescriptorPool &&o) noexcept
            : device(o.device), descriptorPool(std::exchange(o.descriptorPool, nullptr)) {}

    VulkanDescriptorPool &operator=(VulkanDescriptorPool &&o) noexcept {
        if (this == &o)
            return *this;
        destroy();
        descriptorPool = std::exchange(o.descriptorPool, nullptr);
        return *this;
    }

    [[nodiscard]] VulkanDescriptorSet allocate(const VulkanDescriptorSetLayout &layout) const;

    [[nodiscard]] inline VkDescriptorPool vk() const { return descriptorPool; }

private:
    const VulkanDevice &device;
    VkDescriptorPool descriptorPool;
};

