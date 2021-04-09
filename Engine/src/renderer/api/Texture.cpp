#include "Texture.h"
#include "GraphicsContext.h"

#include "Engine/src/core/assets/RawImage.h"

#include "Engine/src/renderer/vulkan/image/VulkanTexture.h"
#include "Engine/src/core/RenderingSystem.h"

#include <cassert>

std::unique_ptr<Renderer::Texture> Renderer::Texture::Create(const std::string &filename) {
    using namespace ChaosEngine;
    RawImage image = RawImage::readImage("textures/" + filename);

    switch (GraphicsContext::currentAPI) {
        case GraphicsAPI::Vulkan:
            return std::make_unique<VulkanTexture>(
                    VulkanTexture::Create(
                            dynamic_cast<const VulkanContext &>(RenderingSystem::GetContext()), image,
                            "textures/" + filename));
        default:
            assert("Invalid Graphics API" && false);
    }
    return nullptr;
}
