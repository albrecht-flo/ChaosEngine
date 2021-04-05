#include "VulkanFramebuffer.h"

#include "VulkanImage.h"
#include "VulkanTexture.h"

#include <stdexcept>
#include <algorithm>
#include <cassert>

static VkFormat getVkFormat(const VulkanContext &context, Renderer::AttachmentFormat format) {
    switch (format) {
        case Renderer::AttachmentFormat::SwapChain:
            return context.getSwapChain().getFormat();
        case Renderer::AttachmentFormat::Auto_Depth:
            return VulkanImage::getDepthFormat(context.getDevice());
        case Renderer::AttachmentFormat::U_R8G8B8A8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        default:
            assert(false);
    }
    return VK_FORMAT_MAX_ENUM;
}

static VulkanImageBuffer
createAttachment(const VulkanContext &context, const Renderer::FramebufferAttachmentInfo &info, uint32_t width,
                 uint32_t height, const std::optional<std::string> &debugName) {
    assert("Can not create image buffer for swapchain!" && info.type != Renderer::AttachmentType::SwapChain);
    std::unique_ptr<VulkanImage> image = nullptr;
    std::unique_ptr<VulkanImageView> imageView = nullptr;
    auto format = getVkFormat(context, info.format);
    switch (info.type) {
        case Renderer::AttachmentType::SwapChain:
            break;
        case Renderer::AttachmentType::Color:
            image = std::make_unique<VulkanImage>(VulkanImage::
                                                  createRawImage(context.getMemory(),
                                                                 width, height, format));
            imageView = std::make_unique<VulkanImageView>(VulkanImageView::
                                                          Create(context.getDevice(), image->vk(), format,
                                                                 VK_IMAGE_ASPECT_COLOR_BIT));
            if (debugName)
                context.setDebugName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t) imageView->vk(),
                                     "Color Attachment View " + *debugName);
            break;
        case Renderer::AttachmentType::Depth:
            image = std::make_unique<VulkanImage>(VulkanImage::
                                                  createDepthBufferImage(context.getMemory(),
                                                                         width, height, format));
            imageView = std::make_unique<VulkanImageView>(VulkanImageView::
                                                          Create(context.getDevice(), image->vk(), format,
                                                                 VK_IMAGE_ASPECT_DEPTH_BIT));
            if (debugName)
                context.setDebugName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t) imageView->vk(),
                                     "Depth Attachment View " + *debugName);
            break;
    }


    return VulkanImageBuffer(context.getDevice(), std::move(*image), std::move(*imageView));
}

// ------------------------------------ Class Members ------------------------------------------------------------------

VulkanFramebuffer VulkanFramebuffer::Create(const VulkanContext &context, VkRenderPass renderPass,
                                            const std::initializer_list<Renderer::FramebufferAttachmentInfo> &infos,
                                            uint32_t width, uint32_t height,
                                            const std::optional<std::string> &debugName) {
    bool swapchain = std::any_of(infos.begin(), infos.end(),
                                 [](auto info) { return info.type == Renderer::AttachmentType::SwapChain; });
    int64_t depthAttachment = std::count_if(infos.begin(), infos.end(),
                                            [](auto info) { return info.type == Renderer::AttachmentType::Depth; });
    assert("Swapchain attachment needs to be bound to attachment 0!" &&
           (!swapchain || infos.begin()->type == Renderer::AttachmentType::SwapChain));
    assert("Only one depth attachment is supported!" && depthAttachment <= 1);
    assert("Framebuffer of size 0x0 it not allowed!" && width != 0 && height != 0);

    std::vector<VulkanImageBuffer> attachments;
    attachments.reserve(infos.size());
    for (auto &info : infos) {
        assert("Depth attachment needs to be the LAST Attachment!" &&
               (info.type != Renderer::AttachmentType::Depth || &info == infos.end() - 1));
        if (info.type != Renderer::AttachmentType::SwapChain) {
            attachments.emplace_back(createAttachment(context, info, width, height, debugName));
        }
    }

    std::vector<VkImageView> attachmentViews;
    attachmentViews.reserve(infos.size());
    int attachmentCounter = 0;
    for (auto &info : infos) {
        if (info.type != Renderer::AttachmentType::SwapChain)
            attachmentViews.emplace_back(attachments[attachmentCounter++].getImageView().vk());
        else
            attachmentViews.emplace_back(context.getSwapChain().getImageViews()[info.index].vk());
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass; // compatible renderpass (attachmentImages match)
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1; // single images

    VkFramebuffer framebuffer;
    if (vkCreateFramebuffer(context.getDevice().vk(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create framebuffer!");
    }

    return VulkanFramebuffer{context.getDevice(), framebuffer, std::move(attachments), swapchain, depthAttachment == 1,
                             width, height};
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanFramebuffer::VulkanFramebuffer(const VulkanDevice &device, VkFramebuffer frameBuffer,
                                     std::vector<VulkanImageBuffer> &&pAttachments, bool swapchainAttached,
                                     bool depthBufferAttached, uint32_t width, uint32_t height)
        : device(device), framebuffer(frameBuffer), attachments(std::move(pAttachments)), attachmentTextures(),
          swapchainAttached(swapchainAttached), depthBufferAttached(depthBufferAttached), width(width), height(height) {
    attachmentTextures.reserve(attachments.size());
    for (auto &attachment: attachments) {
        attachmentTextures.emplace_back(
                VulkanTexture(device, nullptr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              attachment.getImageView().vk(),
                              VulkanSampler::create(device, VK_FILTER_LINEAR)
                ));
    }
}

VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer &&o) noexcept
        : device(o.device), framebuffer(std::exchange(o.framebuffer, nullptr)),
          attachments(std::move(o.attachments)), attachmentTextures(std::move(o.attachmentTextures)),
          swapchainAttached(o.swapchainAttached), depthBufferAttached(o.depthBufferAttached),
          width(o.width), height(o.height) {}

VulkanFramebuffer::~VulkanFramebuffer() { destroy(); }


VulkanFramebuffer &VulkanFramebuffer::operator=(VulkanFramebuffer &&o) noexcept {
    if (this == &o)
        return *this;
    destroy();
    framebuffer = std::exchange(o.framebuffer, nullptr);
    attachments = std::move(o.attachments);
    swapchainAttached = o.swapchainAttached;
    depthBufferAttached = o.depthBufferAttached;
    attachmentTextures = std::move(o.attachmentTextures);
    width = o.width;
    height = o.height;
    return *this;
}

void VulkanFramebuffer::destroy() {
    if (framebuffer != nullptr)
        vkDestroyFramebuffer(device.vk(), framebuffer, nullptr);
}

const Renderer::Texture &VulkanFramebuffer::getAttachmentTexture(Renderer::AttachmentType type, uint32_t index) const {
    assert("Swapchain can not be read!" && type != Renderer::AttachmentType::SwapChain);
    assert("Depth-Attachment is NOT present!" && (type != Renderer::AttachmentType::Depth || depthBufferAttached));
    if (type == Renderer::AttachmentType::Depth && depthBufferAttached)
        return attachmentTextures.back();

    return attachmentTextures[index];
}

