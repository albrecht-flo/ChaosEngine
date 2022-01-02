#pragma once

#include "core/assets/RawImage.h"

#include <string>
#include <memory>
#include <optional>

namespace Renderer {
    class Texture {
    public:
        virtual ~Texture() = default;

        /**
         * Creates a texture from an image file to be used as a texture attachment in a shader.
         * @param filename the path of the image in the `textures/` asset directory
         * @param desiredFormat the format the image should be loaded to
         * @return Created texture
         */
        static std::unique_ptr<Texture> Create(const std::string &filename,
                                               ChaosEngine::ImageFormat desiredFormat = ChaosEngine::ImageFormat::R8G8B8A8);

        static std::unique_ptr<Texture> Create(const ChaosEngine::RawImage &rawImage,
                                               const std::optional<std::string> &debugName = std::nullopt);
    };
}



