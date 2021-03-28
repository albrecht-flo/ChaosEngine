#include "VulkanTexture.h"

#include <utility>

#include "VulkanSampler.h"

#include "VulkanImage.h"

VulkanTexture
VulkanTexture::createTexture(const VulkanDevice &device, const VulkanMemory &vulkanMemory,
                             const std::string &filename) {
    auto image = VulkanImage::createFromFile(device, vulkanMemory, filename);
    VulkanImageView imageView = VulkanImageView::Create(device, image.vk(), VK_FORMAT_R8G8B8A8_UNORM,
                                                        VK_IMAGE_ASPECT_COLOR_BIT);
    VulkanSampler sampler = VulkanSampler::create(device);

    return VulkanTexture{device, std::make_shared<VulkanImage>(std::move(image)),
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, std::move(imageView), std::move(sampler)};
}

VulkanTexture::VulkanTexture(const VulkanDevice &device, std::shared_ptr<VulkanImage> image,
                             VkImageLayout imageLayout, VulkanImageView &&imageView, VulkanSampler &&sampler)
        : device(device), image(std::move(image)), imageLayout(imageLayout),
          imageView(std::move(imageView)), sampler(std::move(sampler)) {}

VulkanTexture::VulkanTexture(const VulkanDevice &device, std::shared_ptr<VulkanImage> image, VkImageLayout imageLayout,
                             VkImageView imageView, VulkanSampler &&sampler)
        : device(device), image(std::move(image)), imageLayout(imageLayout),
          imageViewVk(imageView), sampler(std::move(sampler)) {}

VulkanTexture::VulkanTexture(VulkanTexture &&o) noexcept
        : device(o.device), image(std::move(o.image)), imageLayout(o.imageLayout),
          imageView(std::move(o.imageView)), imageViewVk(o.imageViewVk), sampler(std::move(o.sampler)) {}

VulkanTexture &VulkanTexture::operator=(VulkanTexture &&o) noexcept {
    if (this == &o)
        return *this;
    image = std::move(o.image);
    imageView = std::move(o.imageView);
    sampler = std::move(o.sampler);
    imageViewVk = o.imageViewVk;
    return *this;
}

