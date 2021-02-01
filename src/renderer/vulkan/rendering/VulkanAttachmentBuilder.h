#pragma once

#include <vulkan/vulkan.h>

#include <cassert>
#include <utility>

enum class AttachmentType {
    Color, Depth
};

struct VulkanAttachmentDescription {

    AttachmentType attachmentType;
    VkAttachmentDescription attachment;
    VkImageLayout attachmentLayout;
};

enum class AttachmentLoadOp {
    Preserve, Clear, Undefined
};

enum class AttachmentStoreOp {
    Store, Undefined
};

class VulkanDevice;

class VulkanAttachmentBuilder {

public:
    explicit VulkanAttachmentBuilder(const VulkanDevice &device, AttachmentType attachmentType);

    ~VulkanAttachmentBuilder() = default;

    VulkanAttachmentDescription build() { return {attachmentType, attachment, attachmentLayout}; }

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

    inline VulkanAttachmentBuilder &loadStore(std::pair<AttachmentLoadOp, AttachmentStoreOp> ops) {
        attachment.loadOp = getVkLoadOp(ops.first);
        attachment.storeOp = getVkStoreOp(ops.second);

        return *this;
    }

    inline VulkanAttachmentBuilder &stencilLoadSave(std::pair<AttachmentLoadOp, AttachmentStoreOp> ops) {
        attachment.stencilLoadOp = getVkLoadOp(ops.first);
        attachment.stencilStoreOp = getVkStoreOp(ops.second);
        return *this;
    }

    inline VulkanAttachmentBuilder &layoutInitFinal(std::pair<VkImageLayout, VkImageLayout> initFinal) {
        attachment.initialLayout = initFinal.first;
        attachment.finalLayout = initFinal.second;
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
        }
    }

    static inline VkAttachmentStoreOp getVkStoreOp(AttachmentStoreOp op) {
        switch (op) {
            case AttachmentStoreOp::Store:
                return VK_ATTACHMENT_STORE_OP_STORE;
            case AttachmentStoreOp::Undefined:
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
    }

private:
    AttachmentType attachmentType;
    VkAttachmentDescription attachment;
    VkImageLayout attachmentLayout;
};
