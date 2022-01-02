#pragma once

#include "renderer/api/RenderPass.h"
#include "TestContext.h"
#include "TestFramebuffer.h"

namespace Renderer::TestRenderer {

    class TestRenderPass : public Renderer::RenderPass {

    public:
        explicit TestRenderPass(const TestContext &context)
                : context(context), framebuffer(TestFramebuffer::Create(context)) {}

        ~TestRenderPass() override = default;

        TestRenderPass(const TestRenderPass &o) = delete;

        TestRenderPass &operator=(const TestRenderPass &o) = delete;

        TestRenderPass(TestRenderPass &&o) noexcept
                : context(o.context), framebuffer(std::move(o.framebuffer)) {}

        TestRenderPass &operator=(TestRenderPass &&o) = delete;

        static TestRenderPass Create(const TestContext &context);

        const TestFramebuffer &getFramebuffer() { return framebuffer; }

    private:
        const TestContext &context;
        TestFramebuffer framebuffer;
    };

}
