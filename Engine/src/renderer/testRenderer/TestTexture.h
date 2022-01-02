#pragma once

#include "renderer/api/Texture.h"
#include "TestContext.h"

namespace Renderer::TestRenderer {

    class TestTexture : public Renderer::Texture {
    public:

        explicit TestTexture(const TestContext &context) : context(context) {}

        ~TestTexture() override = default;

        TestTexture(const TestTexture &o) = delete;

        TestTexture &operator=(const TestTexture &o) = delete;

        TestTexture(TestTexture &&o) noexcept = default;

        TestTexture &operator=(TestTexture &&o) noexcept {
            if (this == &o)
                return *this;
            return *this;
        }

        static TestTexture Create(const TestContext &context);

    private:
        const TestContext &context;
    };

}
