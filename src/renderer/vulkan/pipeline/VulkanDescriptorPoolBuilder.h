#pragma  once

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanDescriptorPool.h"

/**
 * This class controls the creation of a new descriptor pool.
 */
class VulkanDescriptorPoolBuilder {
    friend VulkanDescriptorPool;
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

