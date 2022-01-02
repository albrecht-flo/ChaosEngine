#include "TestFramebuffer.h"
#include "renderer/api/Texture.h"

using namespace Renderer::TestRenderer;

TestFramebuffer TestFramebuffer::Create(const TestContext &context) {
    return TestFramebuffer(context);
}

const Renderer::Texture &
TestFramebuffer::getAttachmentTexture(Renderer::AttachmentType /*type*/, uint32_t /*index*/) const {
    return texture;
}
