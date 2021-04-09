#include "Texture.h"
#include "GraphicsContext.h"

#include "Engine/src/renderer/vulkan/image/VulkanTexture.h"
#include "Engine/src/core/RenderingSystem.h"

#include <cassert>

using namespace Renderer;

std::unique_ptr<Texture> Texture::Create(const ChaosEngine::RawImage &rawImage,
                                         const std::optional<std::string> &debugName) {
    switch (GraphicsContext::currentAPI) {
        case GraphicsAPI::Vulkan:
            return std::make_unique<VulkanTexture>(
                    VulkanTexture::Create(
                            dynamic_cast<const VulkanContext &>(RenderingSystem::GetContext()), rawImage, debugName));
        default:
            assert("Invalid Graphics API" && false);
            return nullptr;
    }
}
