#pragma once

#include "renderer/api/Framebuffer.h"
#include "TestContext.h"
#include "TestTexture.h"

namespace Renderer::TestRenderer {

    class TestFramebuffer : public Renderer::Framebuffer {
    private:
        explicit TestFramebuffer(const TestContext &context)
                : context(context), texture(context) {}

    public:

        TestFramebuffer(const TestFramebuffer &o) = delete;

        ~TestFramebuffer() override = default;

        TestFramebuffer &operator=(const TestFramebuffer &o) = delete;

        TestFramebuffer(TestFramebuffer &&o) noexcept = default;

        TestFramebuffer &operator=(TestFramebuffer &&o) noexcept = default;

        static TestFramebuffer Create(const TestContext &context);

        // ------------------------------------ Class Members --------------------------------------------------------------

        [[nodiscard]] uint32_t getWidth() const override { return 400; }

        [[nodiscard]] uint32_t getHeight() const override { return 400; }

        [[nodiscard]] const Renderer::Texture &
        getAttachmentTexture(Renderer::AttachmentType type, uint32_t index) const override;

    private:
        const TestContext &context;
        TestTexture texture;
    };

}
