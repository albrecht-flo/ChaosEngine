#pragma once

#include <vulkan/vulkan.h>

#include <string>

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "src/renderer/vulkan/memory/VulkanMemory.h"
#include "src/renderer/vulkan/image/VulkanImageView.h"
#include "src/renderer/vulkan/image/VulkanImage.h"

class VulkanTexture {
private:
    VulkanTexture(const VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory, VulkanImageView &&imageView,
                  VkSampler sampler);

    void destroy();

public:
    explicit VulkanTexture(const VulkanDevice &device);

    ~VulkanTexture();

    VulkanTexture(const VulkanTexture &o) = delete;

    VulkanTexture &operator=(const VulkanTexture &o) = delete;

    VulkanTexture(VulkanTexture &&o) noexcept;

    VulkanTexture &operator=(VulkanTexture &&o) noexcept;

    static VulkanTexture
    createTexture(const VulkanDevice &device, VulkanMemory &vulkanMemory, const std::string &filename);

    inline VkImage getImage() const { return image; }

    inline const VulkanImageView &getImageView() const { return imageView; }

    inline VkSampler getSampler() const { return sampler; }

private:
    const VulkanDevice &device;
    VkImage image;
    VkDeviceMemory imageMemory;
    VulkanImageView imageView;
    VkSampler sampler;
};