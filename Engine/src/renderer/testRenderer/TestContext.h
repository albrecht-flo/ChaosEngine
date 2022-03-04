#pragma once

#include "renderer/api/GraphicsContext.h"

namespace Renderer::TestRenderer {
    class TestContext : public Renderer::GraphicsContext {
    public:
        explicit TestContext(Window &window) : window(window) {}

        ~TestContext() override = default;

        TestContext(const TestContext &o) = delete;

        TestContext &operator=(const TestContext &o) = delete;

        TestContext(TestContext &&o) = delete;

        TestContext &operator=(TestContext &&o) = delete;

// ------------------------------- Inherited required members ----------------------------------------------------------

        void beginFrame() const override;

        bool flushCommands() override;

        void destroyBuffered(std::unique_ptr<BufferedGPUResource> resource) override;

        void tickFrame() override;

        void waitIdle() override;

    private:
        // Context
        const Window &window;
    };
}



