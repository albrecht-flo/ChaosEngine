#include "FontManager.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdexcept>

using namespace ChaosEngine;

FontManager::FontManager() {
    if (FT_Init_FreeType(&freetype)) {
        throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");
    }
}

FontManager::~FontManager() {
    FT_Done_FreeType(freetype);
}

std::optional<std::shared_ptr<Font>>
FontManager::getFont(const std::string &name, ChaosEngine::FontStyle style, float size, float resolution) const {
    // Linear search, because this should never be a huge vector.
    for (const auto &font: loadedFonts) {
        if (font->size == size) { // First filter by size for performance.
            if (font->name == name && font->style == style && font->resolution == resolution) {
                return font;
            }
        }
    }

    return std::nullopt;
}

std::shared_ptr<Font>
FontManager::loadFont(const std::string &name, const std::string &ttfFile, ChaosEngine::FontStyle style,
                      double size, double resolution) {

    auto existingInstance = getFont(name, style, (float) size, (float) resolution);
    if (existingInstance)
        return *existingInstance;

    auto font = Font::Create(freetype, name, ttfFile, style, size, resolution);
    loadedFonts.emplace_back(font);

    fontsMeta.emplace_back(FontMeta{font->getName(), font->getStyle(), font->getSize(), font->getResolution()});
    return font;
}
