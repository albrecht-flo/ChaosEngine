#include "TestContext.h"
#include "core/Utils/Logger.h"
#include "vendor/platform.h"

using namespace Renderer::TestRenderer;

void TestContext::beginFrame() const {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

bool TestContext::flushCommands() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

void TestContext::destroyBuffered(std::unique_ptr<BufferedGPUResource> /*resource*/) {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestContext::tickFrame() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}

void TestContext::waitIdle() {
    LOG_DEBUG(__PRETTY_FUNCTION__);
}
