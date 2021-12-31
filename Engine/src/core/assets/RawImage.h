#pragma once

#include <string>
#include <memory>

namespace ChaosEngine {
    // TODO: Mappings of unsupported image Formats
    // Note: 3 Component Image Formats are often not supported on Vulkan Hardware
    enum class ImageFormat {
        R8, R8G8B8A8,
        Rf32, Rf32Gf32Bf32Af32,
    };

    class RawImage {
    public:
        RawImage(std::unique_ptr<unsigned char[]> pixels, uint32_t width, uint32_t height, uint64_t size,
                 ImageFormat format);

        ~RawImage();

        RawImage(const RawImage &o) = delete;

        RawImage &operator=(const RawImage &o) = delete;

        RawImage(RawImage &&o) noexcept
                : pixels(std::move(o.pixels)), width(o.width), height(o.height), size(o.size), format(o.format) {}

        RawImage &operator=(RawImage &&o) noexcept {
            if (&o == this)
                return *this;
            pixels = std::move(o.pixels);
            width = o.width;
            height = o.height;
            size = o.size;
            format = o.format;
            return *this;
        }

// ----------------------------- Static Create Functions ---------------------------------------------------------------

        static RawImage readImage(const std::string &filename, ImageFormat desiredFormat);

// ------------------------------------ Class Members ------------------------------------------------------------------

        [[nodiscard]] unsigned char *getPixels() const { return pixels.get(); }

        [[nodiscard]] uint32_t getWidth() const { return width; }

        [[nodiscard]] uint32_t getHeight() const { return height; }

        [[nodiscard]] uint64_t getSize() const { return size; }

    private:
        std::unique_ptr<unsigned char[]> pixels;
        uint32_t width;
        uint32_t height;
        uint64_t size;
        ImageFormat format;
    };

}

