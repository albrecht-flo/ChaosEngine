#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanAttachmentBuilder.h"

class VulkanDevice;

/** This class wraps the vulkan render pass and its creation.
 *  Note: Currently only one main graphics sub pass is supported.
 *
 *  This render pass will wait for previous stages to finish writing to its attachments through sub pass dependencies.
 */
class VulkanRenderPass {
    // TODO: Create Frambuffers from RenderPasses, allowes for basic validity checks
private:
    VulkanRenderPass(const VulkanDevice &device, VkRenderPass renderPass);

    void destroy();

public:
    ~VulkanRenderPass();

    VulkanRenderPass(const VulkanRenderPass &o) = delete;

    VulkanRenderPass &operator=(const VulkanRenderPass &o) = delete;

    VulkanRenderPass(VulkanRenderPass &&o) noexcept;

    VulkanRenderPass &operator=(VulkanRenderPass &&o) = delete;

    static VulkanRenderPass
    Create(const VulkanDevice &device, std::vector<VulkanAttachmentDescription> attachmentDescriptions);

    [[nodiscard]] inline VkRenderPass vk() const { return renderPass; };

private:
    const VulkanDevice &device;
    VkRenderPass renderPass;
};