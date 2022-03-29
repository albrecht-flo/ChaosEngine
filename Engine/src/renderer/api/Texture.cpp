#include "Texture.h"
#include "GraphicsContext.h"

#include "renderer/vulkan/image/VulkanTexture.h"
#include "renderer/testRenderer/TestTexture.h"
#include "Engine/src/core/renderSystem/RenderingSystem.h"
#include "core/utils/Logger.h"

#include <cassert>

using namespace Renderer;
using namespace Renderer::TestRenderer;

std::unique_ptr<Texture>
Texture::Create(const std::string &filename, const ChaosEngine::ImageFormat desiredFormat) {
    LOG_INFO("Loading texture {}", filename);
    ChaosEngine::RawImage image = ChaosEngine::RawImage::readImage("textures/" + filename, desiredFormat);
    return Create(image, "textures/" + filename);
}


std::unique_ptr<Texture> Texture::Create(const ChaosEngine::RawImage &rawImage,
                                         const std::optional<std::string> &debugName) {
    switch (GraphicsContext::currentAPI) {
        case GraphicsAPI::Vulkan:
            return std::make_unique<VulkanTexture>(
                    VulkanTexture::Create(
                            dynamic_cast<const VulkanContext &>(ChaosEngine::RenderingSystem::GetContext()),
                            rawImage, debugName));
        case GraphicsAPI::Test:
            return std::make_unique<TestTexture>(
                    TestTexture::Create(dynamic_cast<const TestContext &>(ChaosEngine::RenderingSystem::GetContext())));
        default:
            assert("Invalid Graphics API" && false);
            return nullptr;
    }
}
