#pragma once

#include "src/renderer/vulkan/context/VulkanDevice.h"

#include <cassert>
#include <utility>

enum class AttachmentType {
    Color, Depth
};

enum class AttachmentLoadOp {
    Preserve, Clear, Undefined
};

enum class AttachmentStoreOp {
    Store, Undefined
};

struct VulkanAttachmentDescription {

    AttachmentType attachmentType;
    VkAttachmentDescription attachment;
    VkImageLayout attachmentLayout;
};

/**
 * This class handle the configuration of the attachments of a render pass.
 *
 * @see VulkanRenderPass
 */
class VulkanAttachmentBuilder {

public:
    explicit VulkanAttachmentBuilder(const VulkanDevice &device, AttachmentType attachmentType);

    ~VulkanAttachmentBuilder() = default;

    inline VulkanAttachmentDescription build() {
        return VulkanAttachmentDescription{attachmentType, attachment, attachmentLayout};
    }

    inline VulkanAttachmentBuilder &format(VkFormat format) {
        attachment.format = format;
        return *this;
    }

    inline VulkanAttachmentBuilder &samples(uint64_t samples) {
        assert(samples == 1 || samples == 2 || samples == 4 || samples == 8 || samples == 16 || samples == 32 ||
               samples == 64);
        auto s = static_cast<VkSampleCountFlagBits>(VK_SAMPLE_COUNT_1_BIT + (samples - 1));
        attachment.samples = s;
        return *this;
    }

    inline VulkanAttachmentBuilder &loadStore(AttachmentLoadOp loadOp, AttachmentStoreOp storeOp) {
        attachment.loadOp = getVkLoadOp(loadOp);
        attachment.storeOp = getVkStoreOp(storeOp);

        return *this;
    }

    inline VulkanAttachmentBuilder &stencilLoadSave(AttachmentLoadOp loadOp, AttachmentStoreOp storeOp) {
        attachment.stencilLoadOp = getVkLoadOp(loadOp);
        attachment.stencilStoreOp = getVkStoreOp(storeOp);
        return *this;
    }

    inline VulkanAttachmentBuilder &layoutInitFinal(VkImageLayout initialLayout, VkImageLayout finalLayout) {
        attachment.initialLayout = initialLayout;
        attachment.finalLayout = finalLayout;
        return *this;
    }

private:
    static inline VkAttachmentLoadOp getVkLoadOp(AttachmentLoadOp op) {
        switch (op) {
            case AttachmentLoadOp::Preserve:
                return VK_ATTACHMENT_LOAD_OP_LOAD;
            case AttachmentLoadOp::Clear:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case AttachmentLoadOp::Undefined:
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            default:
                assert("Unknown load op");
        }
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    static inline VkAttachmentStoreOp getVkStoreOp(AttachmentStoreOp op) {
        switch (op) {
            case AttachmentStoreOp::Store:
                return VK_ATTACHMENT_STORE_OP_STORE;
            case AttachmentStoreOp::Undefined:
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            default:
                assert("Unknown store op");
        }
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

private:
    AttachmentType attachmentType;
    VkAttachmentDescription attachment;
    VkImageLayout attachmentLayout;
};
