#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "src/renderer/vulkan/memory/VulkanMemory.h"
#include "src/renderer/vulkan/image/VulkanImageView.h"

// TODO: massive refactor to be a RAII Wrapper + Builder
class VulkanImage {
public:
    static VkImage createFromFile(const VulkanDevice &device, VulkanMemory &vulkanMemory, const std::string &filename,
                                  VkDeviceMemory &imageMemory/*TEMP*/);

    static VkImage
    createRawImage(const VulkanDevice &device, VulkanMemory &vulkanMemory, uint32_t width, uint32_t height,
                   VkFormat format,
                   VkDeviceMemory &imageMemory/*TEMP*/);

    static VkImage
    createDepthBufferImage(const VulkanDevice &device, const VulkanMemory &vulkanMemory, uint32_t width,
                           uint32_t height,
                           VkFormat depthFormat, VkDeviceMemory &depthImageMemory/*TEMP*/);

    static void destroy(const VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory);

    static VkFormat getDepthFormat(const VulkanDevice &device);

private:
    static void
    createImage(const VulkanDevice &device, const VulkanMemory &vulkanMemory, uint32_t widht, uint32_t height,
                VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image,
                VkDeviceMemory &imageMemory);

    static void
    transitionImageLayout(VulkanMemory &vulkanMemory, VkImage image, VkFormat format, VkImageLayout oldLayout,
                          VkImageLayout newLayout);

    static bool hasStencilComponent(VkFormat format);
};


class VulkanImageBuffer {
public:
    [[deprecated]] explicit VulkanImageBuffer(const VulkanDevice &device)
            : device(device), image(VK_NULL_HANDLE), memory(VK_NULL_HANDLE), imageView(device) {}// TODO: Remove

    VulkanImageBuffer(const VulkanDevice &device, VkImage &&image, VkDeviceMemory &&memory, VulkanImageView &&imageView)
            : device(device), image(image), memory(memory), imageView(std::move(imageView)) {}

    ~VulkanImageBuffer() { destroy(); }

    VulkanImageBuffer(const VulkanImageBuffer &o) = delete;

    VulkanImageBuffer &operator=(const VulkanImageBuffer &o) = delete;

    VulkanImageBuffer(VulkanImageBuffer &&o) noexcept: device(o.device), image(std::exchange(o.image, nullptr)),
                                                       memory(std::exchange(o.memory, nullptr)),
                                                       imageView(std::move(o.imageView)) {}

    VulkanImageBuffer &operator=(VulkanImageBuffer &&o) noexcept {
        if (this == &o)
            return *this;
        destroy();
        image = std::exchange(o.image, nullptr);
        memory = std::exchange(o.memory, nullptr);
        imageView = std::move(o.imageView);
        return *this;
    };

    [[nodiscard]] inline VkImage getImage() const { return image; }

    [[nodiscard]] inline const VulkanImageView &getImageView() const { return imageView; }

private:
    void destroy() { VulkanImage::destroy(device, image, memory); }

private:
    const VulkanDevice &device;
    VkImage image;
    VkDeviceMemory memory;
    VulkanImageView imageView;
};