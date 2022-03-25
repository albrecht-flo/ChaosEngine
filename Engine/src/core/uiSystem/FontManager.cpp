#include "FontManager.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "stb_image_write.h"

#include <cassert>

#include "Engine/src/core/assets/AssetLoader.h"
#include "Engine/src/core/utils/Logger.h"

using namespace ChaosEngine;

std::shared_ptr<Font> FontManager::Create(const std::vector<FontManager::FontDefinition> &fontParts) {
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

    FT_UInt cIndex = 0;
    uint32_t i = 0;
    auto ftChar = FT_Get_First_Char(face, &cIndex);
    const uint32_t yBaseOffset = padding * width;
    const uint32_t xBaseOffset = padding;
    while (ftChar != 0) {
        LOG_INFO("Found character {} in font", ftChar);
        if (FT_Load_Char(face, ftChar, FT_LOAD_RENDER)) {
            throw std::runtime_error("ERROR::FREETYPE: Failed to load glyph");
        }
        if (face->glyph->bitmap.buffer != nullptr) {
            LOG_INFO("Rendered character {} in font. {}x{}", ftChar, face->glyph->bitmap.width,
                     face->glyph->bitmap.rows);
            const uint32_t yOffset = yBaseOffset + (i / mapSize) * (glyphSize * width) +
                                     (i / mapSize) * padding * width;
            const uint32_t xOffset = xBaseOffset + (i % mapSize) * glyphSize +
                                     (i % mapSize) * padding;
            const uint32_t baseOffset = yOffset + xOffset;
            for (uint32_t row = 0; row < face->glyph->bitmap.rows; ++row) {
                const uint32_t offset = baseOffset + row * width;
                const uint32_t glyphBufferOffset = row * face->glyph->bitmap.width;
                std::memcpy(mapBuffer.get() + offset, face->glyph->bitmap.buffer + glyphBufferOffset,
                            face->glyph->bitmap.width);
            }
        } else {
            LOG_INFO("Skipping non renderable character {} in font", ftChar);
        }
        // Next
        ftChar = FT_Get_Next_Char(face, ftChar, &cIndex);
        ++i;
    }
    // Append fallback Glyph
    {
        const uint32_t yOffset = yBaseOffset / 2 + (i / mapSize) * (glyphSize * width) +
                                 (i / mapSize) * padding * width;
        const uint32_t xOffset = xBaseOffset + (i % mapSize) * glyphSize +
                                 (i % mapSize) * padding;
        const uint32_t baseOffset = yOffset + xOffset;
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
                                               cell > glyphSize - localPadding - borderWidth) ? 255 : 0;
                }
            }
        }
    }

    // TODO: Glyph boxes and test if they align
    // TODO: Placeholder
    // TEST ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    for (int i = 0; i < 48; ++i) {
//        for (int j = 0; j < 48; ++j) {
//            mapBuffer.get()[i*width + j] = 255;
//        }
//    }
    // TEST ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // TEST
    stbi_write_png("out.png", width, width, 1, mapBuffer.get(), width);

    /*
    // Load meta file
    auto metaFile = AssetLoader::loadString(ttfFile + ".meta");
    std::vector<int> characters;
    std::istringstream iss(metaFile);
    for (std::string line; std::getline(iss, line);) {
        char *end = nullptr;
        long car = std::strtol(line.c_str(), &end, 10);
        if (end != line.c_str()) // Skip header line
            characters.push_back(car);
        if(car > 40)
            break;
    }
    // Check if meta information contains errors / inconsistencies
    LOG_INFO("Meta info contains {} glyphs", characters.size());
    for (int c: characters) {
        auto res = stbtt_FindGlyphIndex(&fontInfo, c);
        if (res == 0) {
            LOG_WARN("Could not find glyph from meta file charset {:x}", (uint32_t) c);
        }
    }
     */

    return nullptr;
}
