#pragma once

#include <string>

namespace ChaosEngine {
    class RawImage {
    public:
        RawImage(unsigned char *pixels, uint32_t width, uint32_t height, uint64_t size)
                : pixels(pixels), width(width), height(height), size(size) {}

        ~RawImage();

        RawImage(const RawImage &o) = delete;

        RawImage &operator=(const RawImage &o) = delete;

        RawImage(RawImage &&o) noexcept
                : pixels(std::exchange(o.pixels, nullptr)), width(o.width), height(o.height), size(o.size) {}

        RawImage &operator=(RawImage &&o) noexcept {
            if (&o == this)
                return *this;
            pixels = std::exchange(o.pixels, nullptr);
            width = o.width;
            height = o.height;
            size = o.size;
            return *this;
        }

// ----------------------------- Static Create Functions ---------------------------------------------------------------

        static RawImage readImage(const std::string &filename);

// ------------------------------------ Class Members ------------------------------------------------------------------

        [[nodiscard]] unsigned char *getPixels() const { return pixels; }

        [[nodiscard]] uint32_t getWidth() const { return width; }

        [[nodiscard]] uint32_t getHeight() const { return height; }

        [[nodiscard]] uint64_t getSize() const { return size; }

    private:
        unsigned char *pixels;
        uint32_t width;
        uint32_t height;
        uint64_t size;
    };

}

