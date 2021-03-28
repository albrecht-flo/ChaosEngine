#pragma once

#include "Engine/src/renderer/api/Texture.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"
#include "Engine/src/renderer/vulkan/memory/VulkanMemory.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"

#include <string>

class VulkanDevice;

// TODO: [Part of VulkanMemory refactoring]
class VulkanTexture : public Renderer::Texture {
public:
    VulkanTexture(const VulkanDevice &device, std::shared_ptr<VulkanImage> image, VkImageLayout imageLayout,
                  VulkanImageView &&imageView, VulkanSampler &&sampler);
    VulkanTexture(const VulkanDevice &device, std::shared_ptr<VulkanImage> image, VkImageLayout imageLayout,
                  VkImageView imageView, VulkanSampler &&sampler);

    ~VulkanTexture() override = default;

    VulkanTexture(const VulkanTexture &o) = delete;

    VulkanTexture &operator=(const VulkanTexture &o) = delete;

    VulkanTexture(VulkanTexture &&o) noexcept;

    VulkanTexture &operator=(VulkanTexture &&o) noexcept;

    static VulkanTexture
    createTexture(const VulkanDevice &device, const VulkanMemory &vulkanMemory, const std::string &filename);

    static VulkanTexture Create(const VulkanContext &context, const std::string &filename) {
        return createTexture(context.getDevice(), context.getMemory(), filename);
    }

    inline VkImageView getImageView() const { return imageView ? imageView->vk() : *imageViewVk; }

    inline VkSampler getSampler() const { return sampler.vk(); }

    inline VkImageLayout getImageLayout() const { return imageLayout; }

private:
    const VulkanDevice &device;
    std::shared_ptr<VulkanImage> image;
    std::optional<VulkanImageView> imageView; // empty in case of reference texture  (Framebuffer attachment)
    std::optional<VkImageView> imageViewVk; // set in case of reference texture (Framebuffer attachment)
    VulkanSampler sampler;
    VkImageLayout imageLayout;
};