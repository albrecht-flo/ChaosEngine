#include "VulkanSampler.h"

#include <stdexcept>

VkSampler VulkanSampler::create(VulkanDevice &device, VkFilter filter) {
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    // Configure filtering
    samplerInfo.magFilter = filter;
    samplerInfo.minFilter = filter;
    // Configure texel addressing
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // we want coordinates form 0 to 1
    // Configure compare operation
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    // Configure mipmapping
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler sampler = {};
    if (vkCreateSampler(device.vk(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create sampler!");
    }
    return sampler;
}

void VulkanSampler::destroy(VulkanDevice &device, VkSampler sampler) {
    vkDestroySampler(device.vk(), sampler, nullptr);
}