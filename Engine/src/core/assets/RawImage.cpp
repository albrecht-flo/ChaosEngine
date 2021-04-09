#include "RawImage.h"

#include "Engine/src/core/Utils/Logger.h"

#include <stb_image.h>

#include <stdexcept>
#include <type_traits>

namespace ChaosEngine {

    RawImage::~RawImage() {
        static_assert(std::is_same<stbi_uc, std::remove_pointer<decltype(pixels)>::type>::value,
                      "STBImage Classes pixels pointer and stb_images stbi_* differ in type!");
        if (pixels != nullptr)
            stbi_image_free(pixels);
    }

    RawImage RawImage::readImage(const std::string &filename) {
        int width, height, channels;

        stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("[STBI] Failed to load " + filename);
        }

        uint64_t size = (long) width * width * 4;

        return RawImage{pixels, static_cast<uint32_t>(width), static_cast<uint32_t>(height), size};
    }
}