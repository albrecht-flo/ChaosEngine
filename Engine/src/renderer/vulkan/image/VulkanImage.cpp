#include "VulkanImage.h"
#include "Engine/src/renderer/vulkan/memory/VulkanBuffer.h"

#include <stdexcept>

using namespace ChaosEngine;

VkFormat VulkanImage::getVkFormat(ImageFormat format) {
    switch (format) {
        case ImageFormat::R8:
            return VK_FORMAT_R8_UNORM;
        case ImageFormat::R8G8B8A8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case ImageFormat::Rf32:
            return VK_FORMAT_R32_SFLOAT;
        case ImageFormat::Rf32Gf32Bf32Af32:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        default:
            assert(("Unknown image format"));
            return VK_FORMAT_R8G8B8A8_UNORM;

    }
}

/* Creates an image for use as a texture from a file. */
VulkanImage
VulkanImage::Create(const VulkanMemory &vulkanMemory, const ChaosEngine::RawImage &rawImage) {

    // Staging buffer to contain data for transfer
    // Creates buffer with usage=transfer_src, host visible and coherent meaning the cpu has access to the memory and changes are immediately known to the driver which will transfer the memmory before the next vkQueueSubmit
    auto stagingBuffer = vulkanMemory.createBuffer(rawImage.getSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                   VMA_MEMORY_USAGE_CPU_TO_GPU);
    // Copy the image to the staging buffer
    vulkanMemory.copyDataToBuffer(stagingBuffer, rawImage.getPixels(), rawImage.getSize(), 0);

    const auto imageFormat = getVkFormat(rawImage.getFormat());
    // Create the image and its memory
    auto image = vulkanMemory.createImage(rawImage.getWidth(), rawImage.getHeight(),
                                          imageFormat,
                                          VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY);

    // Transition the image to the transfer destination layout
    transitionImageLayout(vulkanMemory, image.vk(), imageFormat,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // Copy image data into image from buffer
    vulkanMemory.copyBufferToImage(stagingBuffer, image, rawImage.getWidth(), rawImage.getHeight());
    // Transfer the image layout to the fragment shader read layout
    transitionImageLayout(vulkanMemory, image.vk(), imageFormat,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    assert("Raw Image and Texture Image differ in dimensions!" &&
           rawImage.getWidth() == image.getWidth() && rawImage.getHeight() == image.getHeight());
    return image;
}

/* Creates an image for depth attachment and sample use. */
VulkanImage VulkanImage::createDepthBufferImage(const VulkanMemory &vulkanMemory, uint32_t width, uint32_t height,
                                                VkFormat depthFormat) {
    auto image = vulkanMemory.createImage(width, height, depthFormat,
                                          VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY, true);
    return image;
}


/* Creates an image for color attachment and sample use. */
VulkanImage
VulkanImage::createRawImage(const VulkanMemory &vulkanMemory, uint32_t width, uint32_t height, VkFormat format) {

    auto image = vulkanMemory.createImage(width, height, format,
                                          VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY, true);

    return image;
}

void
VulkanImage::transitionImageLayout(const VulkanMemory &vulkanMemory, VkImage image, VkFormat format,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout) {
    // TODO: Refactor to a more convenient and understandable solution
    vulkanMemory.getTransferCommandPool().runInSingeTimeCommandBuffer([&](VkCommandBuffer commandBuffer) {

        // Barriers are usually used for access synchronization but can be used for data transitions
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        // Depth images need other parameters as texture images
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (hasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        barrier.subresourceRange.baseMipLevel = 0; // no mipmapping
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0; // single layer
        barrier.subresourceRange.layerCount = 1;

        // The source and destination staged need to be set according to the layout transitions
        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // pseudo stage to transfer data
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = 0;

            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        } else {
            throw std::invalid_argument("VULKAN: unsupported layout transition");
        }


        vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage,
                             0,
                             0, nullptr, 0, nullptr,
                             1, &barrier);

    });
}


/* Helper function to find out if a format has a stencil component. */
bool VulkanImage::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

/* Helper function to get a suitable depth format. */
VkFormat VulkanImage::getDepthFormat(const VulkanDevice &device) {
    return device.findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
             VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
