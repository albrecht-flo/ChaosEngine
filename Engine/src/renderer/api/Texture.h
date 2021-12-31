#pragma once

#include "Engine/src/core/assets/RawImage.h"

#include <string>
#include <memory>
#include <optional>

namespace Renderer {
    class Texture {
    public:
        virtual ~Texture() = default;

        static std::unique_ptr<Texture> Create(const std::string &filename) {// NEXT: Format handling
            ChaosEngine::RawImage image = ChaosEngine::RawImage::readImage("textures/" + filename,
                                                                           ChaosEngine::ImageFormat::R8G8B8A8);
            return Create(image, "textures/" + filename);
        }

        static std::unique_ptr<Texture> Create(const ChaosEngine::RawImage &rawImage,
                                               const std::optional<std::string> &debugName = std::nullopt);
    };
}



