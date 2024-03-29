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

void TestRenderer::beginFrame() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::endFrame() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::beginScene(const glm::mat4 &/*viewMatrix*/, const CameraComponent &/*camera*/) {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::endScene() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::beginUI(const glm::mat4 &/*viewMatrix*/) {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::endUI() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::beginTextOverlay(const glm::mat4 &/*viewMatrix*/) {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::endTextOverlay() {
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

void TestRenderer::drawText(const Buffer &/*vertexBuffer*/, const Buffer &/*indexBuffer*/,
                            uint32_t /*indexCount*/, uint32_t /*indexOffset*/,
                            const glm::mat4 &/*modelMat*/, const MaterialInstance &/*materialInstance*/) {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestRenderer::drawUI(const glm::mat4 &/*viewMatrix*/, const Renderer::RenderMesh &/*mesh*/,
                          const Renderer::MaterialInstance &/*material*/) {
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

void TestRenderer::drawSceneDebug(const glm::mat4 &/*viewMat*/, const CameraComponent &/*camera*/,
                                  const Renderer::DebugRenderData &/*debugRenderData*/) {

    LOG_DEBUG(__PRETTY_FUNCTION__);
}
