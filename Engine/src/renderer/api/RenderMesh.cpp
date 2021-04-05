#include "RenderMesh.h"
#include "GraphicsContext.h"
#include "Engine/src/renderer/vulkan/api/VulkanRenderMesh.h"


#include <cassert>

using namespace Renderer;

std::unique_ptr<RenderMesh>
Renderer::RenderMesh::Create(std::unique_ptr<Buffer> &&vertexBuffer, std::unique_ptr<Buffer> &&indexBuffer,
                             size_t indexCount) {
    switch (GraphicsContext::currentAPI) {
        case GraphicsAPI::Vulkan: {
            auto *vBuffer = dynamic_cast<VulkanBuffer *>(vertexBuffer.release());
            auto *iBuffer = dynamic_cast<VulkanBuffer *>(indexBuffer.release());
            return std::make_unique<VulkanRenderMesh>(std::move(*vBuffer), std::move(*iBuffer),
                                                      static_cast<uint32_t>(indexCount));
        }
        default:
            assert("Invalid Graphics API" && false);
    }
    return nullptr;
}
