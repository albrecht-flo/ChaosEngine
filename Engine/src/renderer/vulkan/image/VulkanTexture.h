#pragma once

#include <vulkan/vulkan.h>

#include <string>

#include "Engine/src/renderer/vulkan/memory/VulkanMemory.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"

class VulkanDevice;

// TODO: [Part of VulkanMemory refactoring]
class VulkanTexture {
private:
    VulkanTexture(const VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory, VulkanImageView &&imageView,
                  VulkanSampler &&sampler);

    void destroy();

public:
    [[nodiscard]]explicit VulkanTexture(const VulkanDevice &device)
            : device(device), image(nullptr), imageMemory(nullptr), imageView(device), sampler(device) {}

    ~VulkanTexture();

    VulkanTexture(const VulkanTexture &o) = delete;

    VulkanTexture &operator=(const VulkanTexture &o) = delete;

    VulkanTexture(VulkanTexture &&o) noexcept;

    VulkanTexture &operator=(VulkanTexture &&o) noexcept;

    static VulkanTexture
    createTexture(const VulkanDevice &device, const VulkanMemory &vulkanMemory, const std::string &filename);

    inline VkImage getImage() const { return image; }

    inline const VulkanImageView &getImageView() const { return imageView; }

    inline VkSampler getSampler() const { return sampler.vk(); }

private:
    const VulkanDevice &device;
    VkImage image;
    VkDeviceMemory imageMemory;
    VulkanImageView imageView;
    VulkanSampler sampler;
};