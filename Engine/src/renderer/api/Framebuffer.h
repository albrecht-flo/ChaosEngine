#pragma once

#include <cstdint>

namespace Renderer {

    class Texture;

    enum class AttachmentType {
        SwapChain, Color, Depth
    };

    enum class AttachmentFormat {
        SwapChain, U_R8G8B8A8, Auto_Depth
    };

    struct FramebufferAttachmentInfo {
        AttachmentType type;
        AttachmentFormat format;
        /**
         * Swapchain Attachment: Identifies the swapchain buffer (Swaphchain is always bound to attachment 0)
         * Depth Attachment: Ignored
         * Color Attachment: attachment index (+1 in shader if swapchain attachment present in framebuffer
         */
        uint32_t index;
    };

    class Framebuffer {
    public:
        virtual ~Framebuffer() = default;

        virtual uint32_t getWidth() const = 0;

        virtual uint32_t getHeight() const = 0;

        virtual const Texture &getAttachmentTexture(AttachmentType type, uint32_t index) const = 0;
    };

}
