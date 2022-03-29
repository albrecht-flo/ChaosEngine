#include "Buffer.h"

#include "GraphicsContext.h"
#include "Engine/src/core/renderSystem/RenderingSystem.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"
#include "Engine/src/renderer/vulkan/memory/VulkanBuffer.h"

#include <cassert>

using namespace Renderer;

static VkBufferUsageFlags getVulkanBufferType(BufferType bufferType) {
    switch (bufferType) {
        case Renderer::BufferType::Vertex:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case Renderer::BufferType::Index:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        default:
            assert("Unknown Buffer Type" && false);
    }
    return 0;
}

std::unique_ptr<Buffer> Renderer::Buffer::Create(const void *data, uint64_t size, BufferType bufferType) {
    switch (GraphicsContext::currentAPI) {
        case GraphicsAPI::Vulkan: {
            auto &memory = dynamic_cast<VulkanContext &>(ChaosEngine::RenderingSystem::GetContext()).getMemory();
            VulkanBuffer buffer = memory.createInputBuffer(size, data, getVulkanBufferType(bufferType));
            return std::make_unique<VulkanBuffer>(std::move(buffer));
        }
        default:
            assert("Invalid Graphics API" && false);
    }
    return nullptr;
}

std::unique_ptr<Buffer> Renderer::Buffer::CreateStreaming(const void *data, uint64_t size, BufferType bufferType) {
    switch (GraphicsContext::currentAPI) {
        case GraphicsAPI::Vulkan: {
            auto &memory = dynamic_cast<VulkanContext &>(ChaosEngine::RenderingSystem::GetContext()).getMemory();
            VulkanBuffer buffer = memory.createInputBuffer(size, data, getVulkanBufferType(bufferType));
            return std::make_unique<VulkanBuffer>(std::move(buffer));
        }
        default:
            assert("Invalid Graphics API" && false);
    }
    return nullptr;
}
