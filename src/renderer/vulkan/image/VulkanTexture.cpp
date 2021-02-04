#include "VulkanTexture.h"

#include "../image/VulkanSampler.h"

#include "src/renderer/vulkan/image/VulkanImage.h"

VulkanTexture
VulkanTexture::createTexture(const VulkanDevice &device, const VulkanMemory &vulkanMemory, const std::string &filename) {
    VkDeviceMemory imageMemory;
    VkImage image = VulkanImage::createFromFile(device, vulkanMemory, filename, imageMemory);
    VulkanImageView imageView = VulkanImageView::Create(device, image, VK_FORMAT_R8G8B8A8_UNORM,
                                                        VK_IMAGE_ASPECT_COLOR_BIT);
    VkSampler sampler = VulkanSampler::create(device);

    return VulkanTexture{device, image, imageMemory, std::move(imageView), sampler};
}

VulkanTexture::VulkanTexture(const VulkanDevice &device)
        : device(device), image(nullptr), imageMemory(nullptr), imageView(device), sampler(nullptr) {}

VulkanTexture::VulkanTexture(const VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory,
                             VulkanImageView &&imageView, VkSampler sampler)
        : device(device), image(image), imageMemory(imageMemory), imageView(std::move(imageView)), sampler(sampler) {}

VulkanTexture::VulkanTexture(VulkanTexture &&o) noexcept
        : device(o.device), image(std::exchange(o.image, nullptr)),
          imageMemory(std::exchange(o.imageMemory, nullptr)),
          imageView(std::move(o.imageView)), sampler(std::exchange(o.sampler, nullptr)) {}

VulkanTexture::~VulkanTexture() {
    destroy();
}

VulkanTexture &VulkanTexture::operator=(VulkanTexture &&o) noexcept {
    if (this == &o)
        return *this;
    destroy();
    image = std::exchange(o.image, nullptr);
    imageMemory = std::exchange(o.imageMemory, nullptr);
    imageView = std::move(o.imageView);
    sampler = std::exchange(o.sampler, nullptr);
    return *this;
}


void VulkanTexture::destroy() {
    VulkanSampler::destroy(device, sampler);
    VulkanImage::destroy(device, image, imageMemory);
}
