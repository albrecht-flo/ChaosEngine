#include "Font.h"

#include "stb_image_write.h"

#include <vector>

#include "Engine/src/renderer/api/Texture.h"
#include "Engine/src/core/assets/AssetLoader.h"
#include "Engine/src/core/utils/Logger.h"

using namespace ChaosEngine;

// For testing
[[maybe_unused]] static void
renderUVRect(const glm::vec2 &uvSize, const glm::vec2 &uvOffset, unsigned char *bitmap, uint32_t size) {
    const glm::uvec2 topLeft = uvOffset * (float) size;
    const glm::uvec2 extend = uvSize * (float) size;

    uint32_t xOffset = topLeft.x;
    uint32_t yOffset = topLeft.y * size;
    for (uint32_t x = 0; x < extend.x; ++x) {
        for (uint32_t y = 0; y < extend.y; ++y) {
            bitmap[xOffset + x + yOffset + y * size] = (x < 1 || x >= extend.x - 1) ? 255 : bitmap[xOffset + x +
                                                                                                   yOffset + y * size];
        }
    }
}

std::shared_ptr<Font>
Font::Create(FT_Library &freetype, const std::string &name, const std::string &ttfFile, FontStyle style,
             double size, double resolution) {
    LOG_INFO("Loading Font from {}, with style {} and size {}pt", ttfFile.c_str(), style, size);

    const double pixelSize = size * resolution / 72;
    const int padding = 16;

    FT_Face face;
    if (FT_New_Face(freetype, ttfFile.c_str(), 0, &face)) {
        throw std::runtime_error("ERROR::FREETYPE: Failed to load font");
    }
    const auto toPixelCoord = [=](double coord) { return coord * pixelSize / face->units_per_EM; };

    const auto mapSize = static_cast<uint32_t>(std::ceil(std::sqrt(face->num_glyphs + 1 /* Fallback Glyph */)));
    const uint32_t width = (uint32_t) (mapSize * std::ceil(pixelSize)) + mapSize * padding;
    std::unique_ptr<unsigned char[]> mapBuffer{new unsigned char[width * width]};
    for (uint32_t i = 0; i < width * width; ++i) {
        mapBuffer.get()[i] = 0;
    }

    if (FT_Set_Pixel_Sizes(face, 0, (uint32_t) pixelSize)) {
        throw std::runtime_error("ERROR::FREETYPE: Failed to set pixel size");
    }

    std::unordered_map<uint32_t, Font::CharacterGlyph> charGlyphs;
    FT_UInt cIndex = 0;
    uint32_t i = 0;
    auto ftChar = FT_Get_First_Char(face, &cIndex);
    while (ftChar != 0) {
        if (FT_Load_Char(face, ftChar, FT_LOAD_RENDER)) {
            throw std::runtime_error("ERROR::FREETYPE: Failed to load glyph");
        }

        FT_GlyphSlot slot = face->glyph;
        const uint32_t yOffset = padding + (uint32_t) ((i / mapSize) * pixelSize) +
                                 (i / mapSize) * padding;
        const uint32_t xOffset = padding + (uint32_t) ((i % mapSize) * pixelSize) +
                                 (i % mapSize) * padding;

        if (slot->bitmap.buffer != nullptr) {
            // Render glyph to bitmap
            const uint32_t memoryOffset = yOffset * width + xOffset;
            for (uint32_t row = 0; row < slot->bitmap.rows; ++row) {
                const uint32_t offset = memoryOffset + row * width;
                const uint32_t glyphBufferOffset = row * slot->bitmap.width;
                std::memcpy(mapBuffer.get() + offset, slot->bitmap.buffer + glyphBufferOffset,
                            slot->bitmap.width);
            }
        }

        charGlyphs.insert_or_assign((uint32_t) ftChar, Font::CharacterGlyph{
                .size = glm::vec2(slot->metrics.width >> 6, slot->metrics.height >> 6),
                .uvSize = glm::vec2((float) slot->bitmap.width / ((float) width),
                                    (float) slot->bitmap.rows / ((float) width)),
                .uvOffset = glm::vec2((float) xOffset / ((float) width),
                                      (float) yOffset / ((float) width)),
                .bearing = glm::vec2(slot->bitmap_left, slot->bitmap_top),
                .advance = static_cast<float>(slot->advance.x >> 6),
        });

        // Next
        ftChar = FT_Get_Next_Char(face, ftChar, &cIndex);
        ++i;
    }
    // Append fallback Glyph
    {
        const uint32_t yOffset = padding / 2 + (uint32_t) ((i / mapSize) * pixelSize) +
                                 (i / mapSize) * padding;
        const uint32_t xOffset = padding + (uint32_t) ((i % mapSize) * pixelSize) +
                                 (i % mapSize) * padding;
        const uint32_t baseOffset = yOffset * width + xOffset;
        const auto localPadding = (uint32_t) std::ceil(pixelSize * 0.1);
        const auto borderWidth = (uint32_t) std::ceil(std::sqrt(pixelSize) / 2.0);
        for (uint32_t row = localPadding; row < pixelSize - localPadding; ++row) {
            if (row < localPadding + borderWidth || row > pixelSize - localPadding - borderWidth) {
                // Top/Bottom
                for (auto cell = localPadding; cell < pixelSize - localPadding; ++cell) {
                    const uint32_t offset = baseOffset + row * width + cell;
                    mapBuffer.get()[offset] = 255;
                }
            } else {
                // Sides
                for (auto cell = localPadding; cell < pixelSize - localPadding; ++cell) {
                    const uint32_t offset = baseOffset + row * width + cell;
                    mapBuffer.get()[offset] = (cell < localPadding + borderWidth ||
                                               cell >= pixelSize - localPadding - borderWidth) ? 255 : 0;
                }
            }
        }
        charGlyphs.insert_or_assign(0, Font::CharacterGlyph{
                .size = glm::vec2(pixelSize - 2 * localPadding, pixelSize - 2 * localPadding),
                .uvSize = glm::vec2((float) (pixelSize - 2 * localPadding) / ((float) width),
                                    (float) (pixelSize - 2 * localPadding) / ((float) width)),
                .uvOffset = glm::vec2((float) (xOffset + localPadding) / ((float) width),
                                      (float) (yOffset + localPadding) / ((float) width)),
                .bearing = glm::vec2(localPadding, pixelSize - 2 * localPadding),
                .advance = static_cast<float>(pixelSize),
        });
    }

    // TEST ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    for (int i = 0; i < width * width; ++i) {
//        mapBuffer.get()[i] /= 2;
//    }
//    for (const auto &x: charGlyphs) {
//        renderUVRect(x.second.uvSize, x.second.uvOffset, mapBuffer.get(), width);
//    }
//    stbi_write_png("out.png", width, width, 1, mapBuffer.get(), width);
    // TEST ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const auto lineHeight = toPixelCoord(face->height);

    FT_Done_Face(face);

    using namespace Renderer;
    auto fontTexture = Texture::Create(RawImage(std::move(mapBuffer), width, width, width * width, ImageFormat::R8),
                                       ttfFile);

    return std::make_shared<Font>(name, style, pixelSize, resolution, lineHeight,
                                  std::move(charGlyphs), std::move(fontTexture));
}
