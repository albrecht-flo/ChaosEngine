#pragma once

#include "Engine/src/renderer/api/Framebuffer.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"
#include "VulkanTexture.h"

#include <vector>
#include <cassert>

class VulkanDevice;

class VulkanFramebuffer : public Renderer::Framebuffer {
private:
    explicit VulkanFramebuffer(const VulkanDevice &device, VkFramebuffer frameBuffer,
                               std::vector<VulkanImageBuffer> &&attachments, bool swapchainAttached,
                               bool depthBufferAttached, uint32_t width, uint32_t height);

public:
    ~VulkanFramebuffer() override;

    VulkanFramebuffer(const VulkanFramebuffer &o) = delete;

    VulkanFramebuffer &operator=(const VulkanFramebuffer &o) = delete;

    VulkanFramebuffer(VulkanFramebuffer &&o) noexcept;

    VulkanFramebuffer &operator=(VulkanFramebuffer &&o) noexcept;

    /**
     * The order of attachment infos passed matters: <br>
     * - first must be swapchain if present
     * - depth must be after swapchain or first if present
     * - all other color attachments must be passed in order (index = 0 + # swapchain/depth attachments)
     * @param context
     * @param renderPass
     * @param infos Swapchain Attachment must to be the first one.
     * @param width
     * @param height
     * @return
     */
    static VulkanFramebuffer Create(const VulkanContext &context, VkRenderPass renderPass,
                                    const std::initializer_list<Renderer::FramebufferAttachmentInfo> &infos,
                                    uint32_t width, uint32_t height);

    // ------------------------------------ Class Members --------------------------------------------------------------

    [[nodiscard]] inline VkFramebuffer vk() const { return framebuffer; }


    [[nodiscard]] uint32_t getWidth() const override { return width; }

    uint32_t getHeight() const override { return height; }

    [[nodiscard]] const Renderer::Texture &
    getAttachmentTexture(Renderer::AttachmentType type, uint32_t index) const {
        assert("Swapchain can not be read!" && type != Renderer::AttachmentType::SwapChain);
        assert("Depth-Attachment is NOT present!" && type != Renderer::AttachmentType::Depth || depthBufferAttached);
        if (type == Renderer::AttachmentType::Depth && depthBufferAttached)
            return attachmentTextures.back();

        return attachmentTextures[index];
    }

private:
    void destroy();

private:
    const VulkanDevice &device;
    VkFramebuffer framebuffer;
    std::vector<VulkanImageBuffer> attachments;
    std::vector<VulkanTexture> attachmentTextures;
    bool swapchainAttached;
    bool depthBufferAttached;
    uint32_t width;
    uint32_t height;
};