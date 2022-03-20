#include "TestRenderer.h"
#include "core/utils/Logger.h"
#include "vendor/platform.h"

using namespace Renderer::TestRenderer;

std::unique_ptr<TestRenderer> TestRenderer::Create(Renderer::GraphicsContext &graphicsContext) {
    LOG_DEBUG(__PRETTY_FUNCTION__);
    auto &context = dynamic_cast<TestContext &>(graphicsContext);
    auto testPass = TestRenderPass::Create(context);
    return std::make_unique<TestRenderer>(context, std::move(testPass));
}

void TestRenderer::setup() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::join() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::beginScene(const glm::mat4 &/*viewMatrix*/, const CameraComponent &/*camera*/) {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::endScene() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::flush() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::requestViewportResize(const glm::vec2 &/*viewportSize*/) {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::draw(const glm::mat4 &/*viewMatrix*/, const RenderComponent &/*renderComponent*/) {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

const Renderer::RenderPass &
TestRenderer::getRenderPassForShaderStage(Renderer::ShaderPassStage /*stage*/) const {
    LOG_DEBUG(__PRETTY_FUNCTION__);
    return testPass;
}

const Renderer::Framebuffer &TestRenderer::getFramebuffer() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
    return testPass.getFramebuffer();
}
