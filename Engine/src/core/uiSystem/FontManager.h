#pragma once

#include "glm/glm.hpp"

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include "Engine/src/renderer/api/Texture.h"

namespace ChaosEngine {

    enum class FontStyle {
        Regular, Italic, Bold
    };

    class Font {
        friend class FontManager;

    private:
        struct CharacterGlyph {
            glm::vec2 size;
            glm::vec2 uvSize;
            glm::vec2 uvOffset;
            glm::vec2 bearing;
            float advance;
        };
    public:
        Font(const std::string &name, FontStyle style, float size, float lineHeight,
             std::unordered_map<uint32_t, CharacterGlyph> &&glyphs,
             std::unique_ptr<Renderer::Texture> &&fontTex) :
                name(name), style(style), size(size), lineHeight(lineHeight),
                glyphs(std::move(glyphs)), fontTex(std::move(fontTex)) {}

        [[nodiscard]] CharacterGlyph getGlyph(uint32_t car) const {
            return glyphs.contains(car) ? glyphs.at(car) : glyphs.at(0);
        }

        [[nodiscard]] Renderer::Texture const *getFontTexture() const { return fontTex.get(); };

        [[nodiscard]] float getSize() const { return size; }

        [[nodiscard]] float getLineHeight() const { return lineHeight; }

    private:
        const std::string &name;
        FontStyle style;
        float size;
        float lineHeight;
        std::unordered_map<uint32_t, CharacterGlyph> glyphs;
        std::unique_ptr<Renderer::Texture> fontTex;
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
