#include "VulkanTexture.h"

#include "VulkanSampler.h"

#include "VulkanImage.h"

VulkanTexture
VulkanTexture::createTexture(const VulkanDevice &device, const VulkanMemory &vulkanMemory,
                             const std::string &filename) {
    VkDeviceMemory imageMemory;
    auto[image, width, height] = VulkanImage::createFromFile(device, vulkanMemory, filename, imageMemory);
    VulkanImageView imageView = VulkanImageView::Create(device, image, VK_FORMAT_R8G8B8A8_UNORM,
                                                        VK_IMAGE_ASPECT_COLOR_BIT);
    VulkanSampler sampler = VulkanSampler::create(device);

    return VulkanTexture{device, image, imageMemory, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, std::move(imageView),
                         std::move(sampler), width, height};
}

VulkanTexture::VulkanTexture(const VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory,
                             VkImageLayout imageLayout, VulkanImageView &&imageView, VulkanSampler &&sampler,
                             uint32_t width, uint32_t height)
        : device(device), image(image), imageMemory(imageMemory), imageLayout(imageLayout),
          imageView(std::move(imageView)), sampler(std::move(sampler)), width(width), height(height) {}

VulkanTexture::VulkanTexture(VulkanTexture &&o) noexcept
        : device(o.device), image(std::exchange(o.image, nullptr)),
          imageMemory(std::exchange(o.imageMemory, nullptr)), imageLayout(o.imageLayout),
          imageView(std::move(o.imageView)), sampler(std::move(o.sampler)), width(o.width), height(o.height) {}

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
    width = o.width;
    height = o.height;
    return *this;
}


void VulkanTexture::destroy() {
    VulkanImage::destroy(device, image, imageMemory);
}
