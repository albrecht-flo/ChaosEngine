#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanDevice;
class VulkanFramebuffer {
private:
    explicit VulkanFramebuffer(const VulkanDevice &device, VkFramebuffer frameBuffer);

public:
    explicit VulkanFramebuffer(const VulkanDevice &device) : device(device), framebuffer(nullptr) {}

    ~VulkanFramebuffer();

    VulkanFramebuffer(const VulkanFramebuffer &o) = delete;

    VulkanFramebuffer &operator=(const VulkanFramebuffer &o) = delete;

    VulkanFramebuffer(VulkanFramebuffer &&o) noexcept;

    VulkanFramebuffer &operator=(VulkanFramebuffer &&o) noexcept;

    static VulkanFramebuffer createFramebuffer(const VulkanDevice &device,
                                               std::vector<VkImageView> attachmentImages,
                                               VkRenderPass renderPass, uint32_t width, uint32_t height);

    [[nodiscard]] inline VkFramebuffer vk() const { return framebuffer; }

private:
    void destroy();

private:
    const VulkanDevice &device;
    VkFramebuffer framebuffer;
};