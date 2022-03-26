#include "FontManager.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "stb_image_write.h"

#include <cassert>

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
    for (int x = 0; x < extend.x; ++x) {
        for (int y = 0; y < extend.y; ++y) {
            bitmap[xOffset + x + yOffset + y * size] = (x < 1 || x >= extend.x - 1) ? 255 : bitmap[xOffset + x +
                                                                                                   yOffset + y * size];
        }
    }
}

std::shared_ptr<Font>
FontManager::Create(const std::string &name, const std::vector<FontManager::FontDefinition> &fontParts) {
    assert("List of font file MUST NOT be empty!" && !fontParts.empty());

    const std::string ttfFile = fontParts[0].ttfFile;
    const auto style = fontParts[0].style;
    const int glyphSize = 48;
    const int padding = 16;
    LOG_INFO("Loading Font from {}, with style {}", ttfFile.c_str(), style);

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");
    }

    FT_Face face;
    if (FT_New_Face(ft, ttfFile.c_str(), 0, &face)) {
        throw std::runtime_error("ERROR::FREETYPE: Failed to load font");
    }

    const auto mapSize = static_cast<uint32_t>(std::ceil(std::sqrt(face->num_glyphs + 1 /* Fallback Glyph */)));
    const uint32_t width = mapSize * glyphSize + mapSize * padding;
    std::unique_ptr<unsigned char[]> mapBuffer{new unsigned char[width * width]};
    for (uint32_t i = 0; i < width * width; ++i) {
        mapBuffer.get()[i] = 0;
    }

    if (FT_Set_Pixel_Sizes(face, 0, glyphSize)) {
        throw std::runtime_error("ERROR::FREETYPE: Failed to set pixel size");
    }

    std::map<wchar_t, Font::CharacterGlyph> charGlyphs;
    FT_UInt cIndex = 0;
    uint32_t i = 0;
    auto ftChar = FT_Get_First_Char(face, &cIndex);
    const uint32_t yBaseOffset = padding;
    const uint32_t xBaseOffset = padding;
    while (ftChar != 0) {
        LOG_INFO("Found character {} in font", ftChar);
        if (FT_Load_Char(face, ftChar, FT_LOAD_RENDER)) {
            throw std::runtime_error("ERROR::FREETYPE: Failed to load glyph");
        }
        const uint32_t yOffset = yBaseOffset + (i / mapSize) * glyphSize +
                                 (i / mapSize) * padding;
        const uint32_t xOffset = xBaseOffset + (i % mapSize) * glyphSize +
                                 (i % mapSize) * padding;
        if (face->glyph->bitmap.buffer != nullptr) {
            LOG_INFO("Rendered character {} in font. {}x{}", ftChar, face->glyph->bitmap.width,
                     face->glyph->bitmap.rows);
            const uint32_t memoryOffset = yOffset * width + xOffset;
            for (uint32_t row = 0; row < face->glyph->bitmap.rows; ++row) {
                const uint32_t offset = memoryOffset + row * width;
                const uint32_t glyphBufferOffset = row * face->glyph->bitmap.width;
                std::memcpy(mapBuffer.get() + offset, face->glyph->bitmap.buffer + glyphBufferOffset,
                            face->glyph->bitmap.width);
            }
        } else {
            LOG_INFO("Skipping non renderable character {} in font", ftChar);
        }
        charGlyphs.insert_or_assign(ftChar, Font::CharacterGlyph{
                .uvSize = glm::vec2((float) face->glyph->bitmap.width / ((float) width),
                                    (float) face->glyph->bitmap.rows / ((float) width)),
                .uvOffset = glm::vec2((float) xOffset / ((float) width),
                                      (float) yOffset / ((float) width)),
                .advance = static_cast<uint32_t>(face->glyph->advance.x),
        });
        // Next
        ftChar = FT_Get_Next_Char(face, ftChar, &cIndex);
        ++i;
    }
    // Append fallback Glyph
    {
        const uint32_t yOffset = yBaseOffset / 2 + (i / mapSize) * glyphSize +
                                 (i / mapSize) * padding;
        const uint32_t xOffset = xBaseOffset + (i % mapSize) * glyphSize +
                                 (i % mapSize) * padding;
        const uint32_t baseOffset = yOffset * width + xOffset;
        const auto localPadding = (uint32_t) std::ceil(glyphSize * 0.1);
        const auto borderWidth = (uint32_t) std::ceil(std::sqrt(glyphSize) / 2.0);
        for (uint32_t row = localPadding; row < glyphSize - localPadding; ++row) {
            if (row < localPadding + borderWidth || row > glyphSize - localPadding - borderWidth) {
                // Top/Bottom
                for (auto cell = localPadding; cell < glyphSize - localPadding; ++cell) {
                    const uint32_t offset = baseOffset + row * width + cell;
                    mapBuffer.get()[offset] = 255;
                }
            } else {
                // Sides
                for (auto cell = localPadding; cell < glyphSize - localPadding; ++cell) {
                    const uint32_t offset = baseOffset + row * width + cell;
                    mapBuffer.get()[offset] = (cell < localPadding + borderWidth ||
                                               cell >= glyphSize - localPadding - borderWidth) ? 255 : 0;
                }
            }
        }
        charGlyphs.insert_or_assign(0, Font::CharacterGlyph{
                .uvSize = glm::vec2((float) (glyphSize - 2 * localPadding) / ((float) width),
                                    (float) (glyphSize - 2 * localPadding) / ((float) width)),
                .uvOffset = glm::vec2((float) (xOffset + localPadding) / ((float) width),
                                      (float) (yOffset + localPadding) / ((float) width)),
                .advance = glyphSize - 2 * localPadding,
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

    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    return std::make_shared<Font>(name, style, glyphSize, std::move(charGlyphs), std::move(mapBuffer));
}
