#pragma once

#include "Engine/src/core/assets/RawImage.h"

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"
#include "Engine/src/renderer/vulkan/memory/VulkanMemory.h"
#include "VulkanImageView.h"

#include <string>

class VulkanImage {
    friend class VulkanMemory;

    friend class VulkanFramebuffer;

private:
    VulkanImage(const VulkanMemory &memory, VkImage image, VmaAllocation imageAllocation,
                uint32_t width, uint32_t height, VkFormat format)
            : memory(memory), image(image), imageAllocation(imageAllocation), width(width), height(height),
              format(format) {}

public:
    ~VulkanImage() { destroy(); }

    VulkanImage(const VulkanImage &o) = delete;

    VulkanImage &operator=(const VulkanImage &o) = delete;

    VulkanImage(VulkanImage &&o) noexcept
            : memory(o.memory), image(std::exchange(o.image, nullptr)),
              imageAllocation(std::exchange(o.imageAllocation, nullptr)),
              width(o.width), height(o.height), format(o.format) {}

    VulkanImage &operator=(VulkanImage &&o) noexcept {
        if (&o == this)
            return *this;
        destroy();
        image = std::exchange(o.image, nullptr);
        imageAllocation = std::exchange(o.imageAllocation, nullptr);
        width = o.width;
        height = o.height;
        format = o.format;
        return *this;
    }

    [[nodiscard]] inline VkImage vk() const { return image; }

    [[nodiscard]] inline uint32_t getWidth() const { return width; }

    [[nodiscard]] inline uint32_t getHeight() const { return height; }

    [[nodiscard]] inline VkFormat getFormat() const { return format; }

public:
    static VulkanImage
    Create(const VulkanMemory &vulkanMemory, const ChaosEngine::RawImage &image);

    static VulkanImage
    createRawImage(const VulkanMemory &vulkanMemory, uint32_t width, uint32_t height, VkFormat format);

    static VulkanImage
    createDepthBufferImage(const VulkanMemory &vulkanMemory, uint32_t width, uint32_t height,
                           VkFormat depthFormat);

    static VkFormat getDepthFormat(const VulkanDevice &device);

private:

    static void
    transitionImageLayout(const VulkanMemory &vulkanMemory, VkImage image, VkFormat format, VkImageLayout oldLayout,
                          VkImageLayout newLayout);

    static bool hasStencilComponent(VkFormat format);

    static VkFormat getVkFormat(ChaosEngine::ImageFormat format);

private:
    void destroy() {
        if (image != nullptr)
            memory.destroyImage(image, imageAllocation);
    }

private:
    const VulkanMemory &memory;
    VkImage image;
    VmaAllocation imageAllocation;
    uint32_t width;
    uint32_t height;
    VkFormat format;
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