#pragma once

#include "glm/glm.hpp"

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace ChaosEngine {

    enum class FontStyle {
        Regular, Italic, Bold
    };

    class Font {
        friend class FontManager;

    private:
        struct CharacterGlyph {
            glm::vec2 uvSize;
            glm::vec2 uvOffset;
            uint32_t advance;
        };
    public:
        Font(const std::string &name, FontStyle style, uint32_t size, std::map<wchar_t, CharacterGlyph> &&glyphs,
             std::unique_ptr<unsigned char[]> &&bitmap) :
                name(name), style(style), size(size), glyphs(std::move(glyphs)), bitmap(std::move(bitmap)) {}

    private:
        const std::string &name;
        FontStyle style;
        uint32_t size;
        std::map<wchar_t, CharacterGlyph> glyphs;
        std::unique_ptr<unsigned char[]> bitmap;
    };

    class FontManager {
    public:
        struct FontDefinition {
            std::string ttfFile;
            FontStyle style;
        };
    public:
        static std::shared_ptr<Font>
        Create(const std::string &name, const std::vector<FontManager::FontDefinition> &fontParts);
    };

}
