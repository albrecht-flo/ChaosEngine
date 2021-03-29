#pragma once

#include "Engine/src/renderer/api/Framebuffer.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"

#include <vector>

class VulkanDevice;

class VulkanImageBuffer;

class VulkanTexture;

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
     * The order of attachment infos passed matters and <b>MUST</b> conform with the following: <br>
     * <ul>
         * <li> first must be swapchain if present </li>
         * <li> depth must be after swapchain or first if present </li>
         * <li> all other color attachments must be passed in order (index = 0 + # swapchain/depth attachments) </li>
     * </ul>
     * @param context
     * @param renderPass
     * @param infos Swapchain Attachment must be the first one and Depth Attachment must be last one (if present).
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

    [[nodiscard]] uint32_t getHeight() const override { return height; }

    [[nodiscard]] const Renderer::Texture &
    getAttachmentTexture(Renderer::AttachmentType type, uint32_t index) const override;

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