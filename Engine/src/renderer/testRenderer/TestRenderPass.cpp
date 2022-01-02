#include "TestRenderPass.h"

using namespace Renderer::TestRenderer;

TestRenderPass TestRenderPass::Create(const Renderer::TestRenderer::TestContext &context) {
    return TestRenderPass{context};
}
