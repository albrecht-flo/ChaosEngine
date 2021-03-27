#pragma once

#include "Engine/src/renderer/api/Texture.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"
#include "Engine/src/renderer/vulkan/memory/VulkanMemory.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"

#include <string>

class VulkanDevice;

// TODO: [Part of VulkanMemory refactoring]
class VulkanTexture : public Renderer::Texture {
private:
    void destroy();

public:
    VulkanTexture(const VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory, VkImageLayout imageLayout,
                  VulkanImageView &&imageView, VulkanSampler &&sampler, uint32_t width, uint32_t height);

    [[deprecated]] explicit VulkanTexture(const VulkanDevice &device)
            : device(device), image(nullptr), imageMemory(nullptr),
              imageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL), imageView(device), sampler(device) {}

    ~VulkanTexture() override;

    VulkanTexture(const VulkanTexture &o) = delete;

    VulkanTexture &operator=(const VulkanTexture &o) = delete;

    VulkanTexture(VulkanTexture &&o) noexcept;

    VulkanTexture &operator=(VulkanTexture &&o) noexcept;

    static VulkanTexture
    createTexture(const VulkanDevice &device, const VulkanMemory &vulkanMemory, const std::string &filename);

    static VulkanTexture Create(const VulkanContext& context, const std::string &filename) {
        return createTexture(context.getDevice(), context.getMemory(), filename);
    }

    inline VkImage getImage() const { return image; }

    inline const VulkanImageView &getImageView() const { return imageView; }

    inline VkSampler getSampler() const { return sampler.vk(); }

    inline VkImageLayout getImageLayout() const { return imageLayout; }

    inline uint32_t getWidth() const { return width; }
    inline uint32_t getHeight() const { return height; }

private:
    const VulkanDevice &device;
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageLayout imageLayout;
    VulkanImageView imageView;
    VulkanSampler sampler;
    uint32_t width;
    uint32_t height;
};