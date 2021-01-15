#include "VulkanTexture.h"

#include "../image/VulkanImage.h"
#include "../image/VulkanImageView.h"
#include "../image/VulkanSampler.h"


VulkanTexture::VulkanTexture() {}

VulkanTexture::VulkanTexture(VkImage image, VkDeviceMemory imageMemory, VkImageView imageView, VkSampler sampler) :
        image(image), imageMemory(imageMemory), imageView(imageView), sampler(sampler) {

}

VulkanTexture::~VulkanTexture() {

}

VulkanTexture
VulkanTexture::createTexture(VulkanDevice &device, VulkanMemory &vulkanMemroy, const std::string &filename) {
    VkDeviceMemory imageMemory;
    VkImage image = VulkanImage::createFromFile(device, vulkanMemroy, filename, imageMemory);
    VkImageView imageView = VulkanImageView::create(device, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    VkSampler sampler = VulkanSampler::create(device);

    return VulkanTexture(image, imageMemory, imageView, sampler);
}

void VulkanTexture::destroy(VulkanDevice &device, VulkanTexture &texture) {
    VulkanSampler::destroy(device, texture.sampler);
    VulkanImageView::destroy(device, texture.imageView);
    VulkanImage::destroy(device, texture.image, texture.imageMemory);
}