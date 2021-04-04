#include "GraphicsContext.h"
#include "Engine/src/renderer/window/Window.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"

#include <cassert>

namespace Renderer {
    GraphicsAPI GraphicsContext::currentAPI = GraphicsAPI::None;

    std::unique_ptr<GraphicsContext> Renderer::GraphicsContext::Create(Window &window, Renderer::GraphicsAPI api) {
        GraphicsContext::currentAPI = api;
        switch (api) {
            case GraphicsAPI::Vulkan:
                return std::make_unique<VulkanContext>(window);
            case GraphicsAPI::None:
                assert("GraphicsAPI wasn't specified correctly");
            default:
                assert("Unsupported Graphics API");
        }
        return nullptr;
    }
}
