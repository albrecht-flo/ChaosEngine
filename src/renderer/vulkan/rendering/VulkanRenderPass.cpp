#include "VulkanRenderPass.h"

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanAttachmentBuilder.h"

#include <stdexcept>
#include <array>

VulkanRenderPass
VulkanRenderPass::Create(const VulkanDevice &device,
                         const std::vector<VulkanAttachmentDescription> &attachmentDescriptions) {
    assert(!attachmentDescriptions.empty());
    std::vector<VkAttachmentReference> colorAttachmentRefs;
    std::vector<VkAttachmentReference> depthAttachmentRefs;

    std::vector<VkAttachmentDescription> attachments(attachmentDescriptions.size());

    for (int i = 0; i < attachmentDescriptions.size(); ++i) {
        VkAttachmentReference attachmentRef = {};
        attachmentRef.attachment = i;
        attachmentRef.layout = attachmentDescriptions[i].attachmentLayout; // layout ~during~ subpass
        if (attachmentDescriptions[i].attachmentType == AttachmentType::Color) {
            colorAttachmentRefs.emplace_back(attachmentRef);
        } else if (attachmentDescriptions[i].attachmentType == AttachmentType::Depth) {
            depthAttachmentRefs.emplace_back(attachmentRef);
        } else {
            assert("Attachment type not supported.");
        }
        attachments.emplace_back(attachmentDescriptions[i].attachment);
    }

    // For the moment we only have 1 subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // graphics not compute
    subpass.colorAttachmentCount = colorAttachmentRefs.size();
    subpass.pColorAttachments = colorAttachmentRefs.data();
    if (depthAttachmentRefs.size() == 1)
        subpass.pDepthStencilAttachment = depthAttachmentRefs.data();

    // Configure subpass dependency
    // We want our subpass to wait for the previous stage to finish reading the color attachment
    std::array<VkSubpassDependency, 1> dependencies = {};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // implicit prior subpass
    dependencies[0].dstSubpass = 0; // 0 is the current sub pass; otherwise must be > srcSubpass except VK_SUBPASS_EXTERNAL
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // want to wait for swap chain to finish reading framebuffer
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // make following color subpasses wait for this one to finish
    dependencies[0].srcAccessMask = 0; // We don't read
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if (!depthAttachmentRefs.empty()) {
        // If the depth image is beeing cleared we need to wait for it (will be accessed first in fragment test stage)
        dependencies[0].srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    // Combine subpasses, dependencies and attachmentDescriptions to render pass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VkRenderPass renderPass{};
    if (vkCreateRenderPass(device.vk(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create render rendering!");
    }


    return VulkanRenderPass(device, renderPass, attachments.size());
}

VulkanRenderPass::VulkanRenderPass(const VulkanDevice &device, VkRenderPass renderPass, int attachmentCount)
        : device(device), renderPass(renderPass), attachmentCount(attachmentCount) {}

VulkanRenderPass::VulkanRenderPass(VulkanRenderPass &&o) noexcept
        : device(o.device), renderPass(std::exchange(o.renderPass, nullptr)), attachmentCount(o.attachmentCount) {}

VulkanRenderPass::~VulkanRenderPass() {
    destroy();
}

void VulkanRenderPass::destroy() {
    if (renderPass != nullptr)
        vkDestroyRenderPass(device.vk(), renderPass, nullptr);
}

VulkanFramebuffer
VulkanRenderPass::createFrameBuffer(const std::vector<VkImageView> &attachmentImages, VkExtent2D extent) const {
    assert(attachmentImages.size() == attachmentCount);
    return VulkanFramebuffer::createFramebuffer(device, attachmentImages, renderPass, extent.width, extent.height);
}
