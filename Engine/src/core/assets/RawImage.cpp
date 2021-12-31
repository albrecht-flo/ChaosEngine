#include "RawImage.h"

#include "Engine/src/core/Utils/Logger.h"

#include <stb_image.h>

#include <stdexcept>
#include <type_traits>

namespace ChaosEngine {


    static int getStbiFormat(ImageFormat format) {
        switch (format) {
            case ImageFormat::R8:
                return STBI_grey;
            case ImageFormat::R8G8B8A8:
                return STBI_rgb_alpha;
            case ImageFormat::Rf32:
                return STBI_grey;
            case ImageFormat::Rf32Gf32Bf32Af32:
                return STBI_rgb_alpha;
            default:
                assert("Unsupported Format!");
                return 4;
        }
    }

    RawImage::~RawImage() {
        static_assert(std::is_same<stbi_uc, decltype(pixels)::element_type>::value,
                      "RawImage::pixels type and stb_images stbi_* differ in type!");
    }

    RawImage RawImage::readImage(const std::string &filename, ImageFormat desiredFormat) {
        int width, height, channels;

        stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, &channels, getStbiFormat(desiredFormat));

        if (!pixels) {
            throw std::runtime_error("[STBI] Failed to load " + filename);
        }
        if (channels != getStbiFormat(desiredFormat)) {
            LOG_WARN("Desired Format and Image format are not the same! For image {} ; desired # channels {} != {}",
                     filename, getStbiFormat(desiredFormat), channels);
        }

        // Conversion of bytes to floats if necessary
        std::unique_ptr<unsigned char[]> finalPixels = nullptr;
        uint64_t size = 0;
        if (desiredFormat == ImageFormat::Rf32 || desiredFormat == ImageFormat::Rf32Gf32Bf32Af32) {
            size = width * height * sizeof(float) *
                   getStbiFormat(desiredFormat); // STBI Format corresponds to the amount of channels
            finalPixels = std::make_unique<unsigned char[]>(size);
            auto *temp = reinterpret_cast<float *>(finalPixels.get());
            for (int64_t i = 0; i < width * height * getStbiFormat(desiredFormat); ++i) {
                temp[i] = static_cast<float>(pixels[i]) / 255.0f;
            }
        } else if (desiredFormat == ImageFormat::R8 || desiredFormat == ImageFormat::R8G8B8A8) {
            size = width * height * getStbiFormat(desiredFormat); // STBI Format corresponds to the amount of channels
            finalPixels = std::make_unique<unsigned char[]>(size);
            std::memcpy(finalPixels.get(), pixels, size);
        } else {
            assert("Unsupported Image Format!");
        }

        stbi_image_free(pixels);

        return RawImage{std::move(finalPixels), static_cast<uint32_t>(width), static_cast<uint32_t>(height), size,
                        desiredFormat};
    }

    RawImage::RawImage(std::unique_ptr<unsigned char[]> pixels, uint32_t width, uint32_t height, uint64_t size,
                       ImageFormat format)
            : pixels(std::move(pixels)), width(width), height(height), size(size), format(format) {
        assert("Pixels MUST not be NULL!" && RawImage::pixels != nullptr);
    }
}