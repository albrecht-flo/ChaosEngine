#include "VulkanTexture.h"

#include "../image/VulkanSampler.h"

#include "src/renderer/vulkan/image/VulkanImage.h"

VulkanTexture
VulkanTexture::createTexture(const VulkanDevice &device, const VulkanMemory &vulkanMemory,
                             const std::string &filename) {
    VkDeviceMemory imageMemory;
    VkImage image = VulkanImage::createFromFile(device, vulkanMemory, filename, imageMemory);
    VulkanImageView imageView = VulkanImageView::Create(device, image, VK_FORMAT_R8G8B8A8_UNORM,
                                                        VK_IMAGE_ASPECT_COLOR_BIT);
    VulkanSampler sampler = VulkanSampler::create(device);

    return VulkanTexture{device, image, imageMemory, std::move(imageView), std::move(sampler)};
}

VulkanTexture::VulkanTexture(const VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory,
                             VulkanImageView &&imageView, VulkanSampler &&sampler)
        : device(device), image(image), imageMemory(imageMemory), imageView(std::move(imageView)),
          sampler(std::move(sampler)) {}

VulkanTexture::VulkanTexture(VulkanTexture &&o) noexcept
        : device(o.device), image(std::exchange(o.image, nullptr)),
          imageMemory(std::exchange(o.imageMemory, nullptr)),
          imageView(std::move(o.imageView)), sampler(std::move(o.sampler)) {}

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
    sampler = std::move(o.sampler);
    return *this;
}


void VulkanTexture::destroy() {
    VulkanImage::destroy(device, image, imageMemory);
}
