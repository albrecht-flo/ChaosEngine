#include "VulkanAttachmentBuilder.h"

#include <renderer/vulkan/context/VulkanSwapChain.h>

#include "Engine/src/renderer/vulkan/image/VulkanImage.h"


VulkanAttachmentBuilder::VulkanAttachmentBuilder(const VulkanContext &context, Renderer::AttachmentType attachmentType)
        : attachmentType(attachmentType) {
    if(attachmentType == Renderer::AttachmentType::Color || attachmentType == Renderer::AttachmentType::SwapChain)
        attachmentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    else
        attachmentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if (attachmentType == Renderer::AttachmentType::Color) {
        attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling so only 1
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear before new frame
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store results instead of discarding them
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // layout ~before~ render rendering
        attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // layout ~after~ render rendering
    } else if (attachmentType == Renderer::AttachmentType::Depth) {
        attachment.format = VulkanImage::getDepthFormat(context.getDevice());
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear before next draw
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // post will need this
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // layout ~before~ render rendering
        attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // layout ~after~ render rendering
    } else if (attachmentType == Renderer::AttachmentType::SwapChain) {
        attachment.format = context.getSwapChain().getFormat();
        attachment.samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling so only 1
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear before new frame
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store results instead of discarding them
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // layout ~before~ render rendering
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // layout ~after~ render rendering
    } else {
        assert("Unsupported attachment type" && false);
    }
}
