#pragma once

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"
#include "Engine/src/renderer/vulkan/memory/VulkanMemory.h"
#include "VulkanImageView.h"

#include <string>

// TODO: massive refactor to be a RAII Wrapper + Builder [Part of VulkanMemory refactoring]
class VulkanImage {
    friend class VulkanFramebuffer;
private:
    VulkanImage(const VulkanDevice &device, VkImage image, VkDeviceMemory imageMemory, uint32_t width, uint32_t height)
            : device(device), image(image), imageMemory(imageMemory), width(width), height(height) {}

public:
    ~VulkanImage() { destroy(); }

    VulkanImage(const VulkanImage &o) = delete;

    VulkanImage &operator=(const VulkanImage &o) = delete;

    VulkanImage(VulkanImage &&o) noexcept
            : device(o.device), image(std::exchange(o.image, nullptr)),
              imageMemory(std::exchange(o.imageMemory, nullptr)),
              width(o.width), height(o.height) {}

    VulkanImage &operator=(VulkanImage &&o) noexcept {
        if (&o == this)
            return *this;
        destroy();
        image = std::exchange(o.image, nullptr);
        imageMemory = std::exchange(o.imageMemory, nullptr);
        width = o.width;
        height = o.height;
        return *this;
    }

    [[nodiscard]] inline VkImage vk() const { return image; }

    [[nodiscard]] inline uint32_t getWidth() const { return width; }

    [[nodiscard]] inline uint32_t getHeight() const { return height; }

public:
    static VulkanImage
    createFromFile(const VulkanDevice &device, const VulkanMemory &vulkanMemory, const std::string &filename);

    static VulkanImage
    createRawImage(const VulkanDevice &device, const VulkanMemory &vulkanMemory, uint32_t width, uint32_t height,
                   VkFormat format);

    static VulkanImage
    createDepthBufferImage(const VulkanDevice &device, const VulkanMemory &vulkanMemory, uint32_t width,
                           uint32_t height, VkFormat depthFormat);

    static VkFormat getDepthFormat(const VulkanDevice &device);

private:
    static void
    createImage(const VulkanDevice &device, const VulkanMemory &vulkanMemory, uint32_t widht, uint32_t height,
                VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image,
                VkDeviceMemory &imageMemory);

    static void
    transitionImageLayout(const VulkanMemory &vulkanMemory, VkImage image, VkFormat format, VkImageLayout oldLayout,
                          VkImageLayout newLayout);

    static bool hasStencilComponent(VkFormat format);

private:
    void destroy() {
        if (image != nullptr) {
            vkDestroyImage(device.vk(), image, nullptr);
            vkFreeMemory(device.vk(), imageMemory, nullptr);
        }
    }

private:
    const VulkanDevice &device;
    VkImage image;
    VkDeviceMemory imageMemory;
    uint32_t width;
    uint32_t height;
};


class VulkanImageBuffer {
public:
    VulkanImageBuffer(const VulkanDevice &device, VulkanImage &&image, VulkanImageView &&imageView)
            : device(device), image(std::move(image)), imageView(std::move(imageView)) {}

    ~VulkanImageBuffer() = default;

    VulkanImageBuffer(const VulkanImageBuffer &o) = delete;

    VulkanImageBuffer &operator=(const VulkanImageBuffer &o) = delete;

    VulkanImageBuffer(VulkanImageBuffer &&o) noexcept: device(o.device), image(std::move(o.image)),
                                                       imageView(std::move(o.imageView)) {}

    VulkanImageBuffer &operator=(VulkanImageBuffer &&o) noexcept {
        if (this == &o)
            return *this;
        image = std::move(o.image);
        imageView = std::move(o.imageView);
        return *this;
    };

    [[nodiscard]] inline VkImage getImage() const { return image.vk(); }

    [[nodiscard]] inline const VulkanImageView &getImageView() const { return imageView; }

    [[nodiscard]] inline uint32_t getWidth() const { return image.getWidth(); }

    [[nodiscard]] inline uint32_t getHeight() const { return image.getHeight(); }

private:
    const VulkanDevice &device;
    VulkanImage image;
    VulkanImageView imageView;
};