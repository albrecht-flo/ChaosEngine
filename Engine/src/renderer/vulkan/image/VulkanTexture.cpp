#include "VulkanTexture.h"

#include <utility>

#include "VulkanSampler.h"

#include "VulkanImage.h"

VulkanTexture
VulkanTexture::Create(const VulkanContext &context, const ChaosEngine::RawImage &rawImage,
                      const std::optional<std::string> &debugName) {
    auto image = VulkanImage::Create(context.getMemory(), rawImage);
    VulkanImageView imageView = VulkanImageView::Create(context.getDevice(), image.vk(), VK_FORMAT_R8G8B8A8_UNORM,
                                                        VK_IMAGE_ASPECT_COLOR_BIT);
    context.setDebugName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t) imageView.vk(), debugName);

    VulkanSampler sampler = VulkanSampler::create(context.getDevice());

    return VulkanTexture{context.getDevice(), std::make_shared<VulkanImage>(std::move(image)),
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, std::move(imageView), std::move(sampler)};
}

VulkanTexture::VulkanTexture(const VulkanDevice &device, std::shared_ptr<VulkanImage> image,
                             VkImageLayout imageLayout, VulkanImageView &&imageView, VulkanSampler &&sampler)
        : device(device), image(std::move(image)), imageView(std::move(imageView)),
          sampler(std::move(sampler)), imageLayout(imageLayout) {}

VulkanTexture::VulkanTexture(const VulkanDevice &device, std::shared_ptr<VulkanImage> image, VkImageLayout imageLayout,
                             VkImageView imageView, VulkanSampler &&sampler)
        : device(device), image(std::move(image)), imageViewVk(imageView),
          sampler(std::move(sampler)), imageLayout(imageLayout) {}

VulkanTexture::VulkanTexture(VulkanTexture &&o) noexcept
        : device(o.device), image(std::move(o.image)), imageView(std::move(o.imageView)), imageViewVk(o.imageViewVk),
          sampler(std::move(o.sampler)), imageLayout(o.imageLayout) {}

VulkanTexture &VulkanTexture::operator=(VulkanTexture &&o) noexcept {
    if (this == &o)
        return *this;
    image = std::move(o.image);
    imageView = std::move(o.imageView);
    sampler = std::move(o.sampler);
    imageViewVk = o.imageViewVk;
    return *this;
}

