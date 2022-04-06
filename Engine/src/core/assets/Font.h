#pragma once

#include "dep/freetype/include/freetype/freetype.h"

#include "glm/glm.hpp"

#include <memory>
#include <string>
#include <unordered_map>

#include "renderer/api/Texture.h"

namespace Renderer { class Texture; }

namespace ChaosEngine {

    enum class FontStyle {
        Regular, Italic, Bold, MAX
    };

    class Font {
        friend class FontManager;

    public:
        struct CharacterGlyph {
            glm::vec2 size;
            glm::vec2 uvSize;
            glm::vec2 uvOffset;
            glm::vec2 bearing;
            float advance;
        };
    public:
        Font(const std::string &name, FontStyle style, double size, double resolution, double lineHeight,
             std::unordered_map<uint32_t, CharacterGlyph> &&glyphs, std::unique_ptr<Renderer::Texture> &&fontTex)
                : name(name), style(style),
                  size((float) size), resolution((float) resolution), lineHeight((float) lineHeight),
                  glyphs(std::move(glyphs)), fontTex(std::move(fontTex)) {}

        [[nodiscard]] const std::string &getName() const { return name; }

        [[nodiscard]] FontStyle getStyle() const { return style; }

        [[nodiscard]] float getSize() const { return static_cast<float>(size); }


        [[nodiscard]] float getResolution() const { return static_cast<float>(resolution); }

        [[nodiscard]] float getLineHeight() const { return static_cast<float>(lineHeight); }

        [[nodiscard]] CharacterGlyph getGlyph(uint32_t car) const {
            return glyphs.contains(car) ? glyphs.at(car) : glyphs.at(0);
        }

        [[nodiscard]] Renderer::Texture const *getFontTexture() const { return fontTex.get(); };

    private:
        // ------------------------------------ Creator ----------------------------------------------------------------
        /// Creates a new font, should be called by the FontManager.
        static std::shared_ptr<Font> Create(FT_Library &freetype, const std::string &name,
                                            const std::string &ttfFile, FontStyle style,
                                            double size, double resolution);

    private:
        const std::string name;
        const FontStyle style;
        const float size;
        const float resolution;
        const float lineHeight;
        const std::unordered_map<uint32_t, CharacterGlyph> glyphs;
        const std::unique_ptr<Renderer::Texture> fontTex;
    };
}
