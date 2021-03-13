#include "GraphicsContext.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"

#include <stdexcept>
#include <cassert>

namespace Renderer {
    GraphicsAPI GraphicsContext::currentAPI = GraphicsAPI::None;

    std::unique_ptr<GraphicsContext> Renderer::GraphicsContext::Create(Window &window, Renderer::GraphicsAPI api) {
        GraphicsContext::currentAPI = api;
        switch (api) {
            case GraphicsAPI::Vulkan:
                return std::make_unique<VulkanContext>(window);
            case GraphicsAPI::None:
                throw std::runtime_error("GraphicsAPI wasn't specified correctly");
        }
        assert(("Unsupported Graphics API", false));
    }
}
