#pragma once

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"
#include "Engine/src/renderer/vulkan/image/VulkanFramebuffer.h"

#include <vector>

class VulkanDevice;

struct VulkanAttachmentDescription;

/** This class wraps a vulkan render pass and its creation.
 *  This render pass will wait for previous stages to finish writing to its attachments through sub pass dependencies.
 *
 *  Note: Currently only one main graphics sub pass is supported.
 */
class VulkanRenderPass {
private:

    void destroy();

public:
    VulkanRenderPass(const VulkanDevice &device, VkRenderPass renderPass, int attachmentCount);

    ~VulkanRenderPass();

    VulkanRenderPass(const VulkanRenderPass &o) = delete;

    VulkanRenderPass &operator=(const VulkanRenderPass &o) = delete;

    VulkanRenderPass(VulkanRenderPass &&o) noexcept;

    VulkanRenderPass &operator=(VulkanRenderPass &&o) = delete;

    static VulkanRenderPass
    Create(const VulkanDevice &device, const std::vector<VulkanAttachmentDescription> &attachmentDescriptions);

    [[nodiscard]] inline VkRenderPass vk() const { return renderPass; }

    [[nodiscard]] VulkanFramebuffer
    createFrameBuffer(const std::initializer_list<VkImageView> &attachmentImages, VkExtent2D extent) const;

private:
    const VulkanDevice &device;
    VkRenderPass renderPass;
    int attachmentCount;
};
