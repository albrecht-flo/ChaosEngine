#include "GraphicsContext.h"
#include "renderer/window/Window.h"
#include "renderer/vulkan/context/VulkanContext.h"
#include "renderer/testRenderer/TestContext.h"

#include <cassert>

namespace Renderer {
    using namespace TestRenderer;
    GraphicsAPI GraphicsContext::currentAPI = GraphicsAPI::None;

    std::unique_ptr<GraphicsContext> Renderer::GraphicsContext::Create(Window &window, Renderer::GraphicsAPI api) {
        GraphicsContext::currentAPI = api;
        switch (api) {
            case GraphicsAPI::Vulkan:
                return std::make_unique<VulkanContext>(window);
            case GraphicsAPI::Test:
                return std::make_unique<TestContext>(window);
            case GraphicsAPI::None:
                assert("GraphicsAPI wasn't specified correctly" && false);
            default:
                assert("Unsupported Graphics API" && false);
        }
        return nullptr;
    }
}
