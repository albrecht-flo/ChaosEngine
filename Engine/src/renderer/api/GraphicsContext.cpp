#include "GraphicsContext.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"

#include <stdexcept>
#include <cassert>

namespace Renderer {
    std::unique_ptr<GraphicsContext> Renderer::GraphicsContext::Create(Window &window, Renderer::GraphicsAPI api) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return std::make_unique<VulkanContext>(window);
            case GraphicsAPI::OpenGl:
                throw std::runtime_error("OpengGl is not yet supported");
        }
        assert(("Unknown Graphics API", false));
    }
}
