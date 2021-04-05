#include "VulkanImage.h"
#include "Engine/src/renderer/vulkan/memory/VulkanBuffer.h"

#include <stb_image.h>

#include <stdexcept>

/* Creates an image for use as a texture from a file. */
VulkanImage
VulkanImage::createFromFile(const VulkanMemory &vulkanMemory, const std::string &filename) {
    int texWidth, texHeight, texChannels;

    stbi_uc *pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    VkDeviceSize imageSize = (long) texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("STBI: Failed to load " + filename);
    }

    // Staging buffer to contain data for transfer
    // Creates buffer with usage=transfer_src, host visible and coherent meaning the cpu has access to the memory and changes are immediately known to the driver which will transfer the memmory before the next vkQueueSubmit
    auto stagingBuffer = vulkanMemory.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                   VMA_MEMORY_USAGE_CPU_TO_GPU);
    // Copy the image to the staging buffer
    vulkanMemory.copyDataToBuffer(stagingBuffer, pixels, imageSize, 0);
    stbi_image_free(pixels); // no longer needed

    // Create the image and its memory
    auto image = vulkanMemory.createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),
                                          VK_FORMAT_R8G8B8A8_UNORM,
                                          VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY);

    // Transition the image to the transfer destination layout
    transitionImageLayout(vulkanMemory, image.vk(), VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // Copy image data into image from buffer
    vulkanMemory.copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(texWidth),
                                   static_cast<uint32_t>(texHeight));
    // Transfer the image layout to the fragment shader read layout
    transitionImageLayout(vulkanMemory, image.vk(), VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return image;
}

/* Creates an image for depth attachment and sample use. */
VulkanImage VulkanImage::createDepthBufferImage(const VulkanMemory &vulkanMemory, uint32_t width, uint32_t height,
                                                VkFormat depthFormat) {
    auto image = vulkanMemory.createImage(width, height, depthFormat,
                                          VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY);
    return image;
}


/* Creates an image for color attachment and sample use. */
VulkanImage
VulkanImage::createRawImage(const VulkanMemory &vulkanMemory, uint32_t width, uint32_t height, VkFormat format) {

    auto image = vulkanMemory.createImage(width, height, format,
                                          VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY);

    return image;
}

/* Transitions the image layout. */
void
VulkanImage::transitionImageLayout(const VulkanMemory &vulkanMemory, VkImage image, VkFormat format,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = vulkanMemory.beginSingleTimeCommands();

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
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::invalid_argument("VULKAN: unsupported layout transition");
    }


    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage,
                         0,
                         0, nullptr, 0, nullptr,
                         1, &barrier);

    vulkanMemory.endSingleTimeCommands(commandBuffer);
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
