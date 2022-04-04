#pragma once

#include <memory>
#include <vector>
#include <optional>

#include <freetype/freetype.h>

#include "Font.h"

namespace ChaosEngine {

    struct FontMeta {
        const std::string name;
        const FontStyle style;
        const float size;
        const float resolution;
    };

    /**
     * This class is a utility class for the asset manager because these resources may live longer than the entities
     * referencing them. While also being very slow to load due to font rendering.
     */
    class FontManager {
    public:
        FontManager();

        ~FontManager();

        /// Loads a font from disk, if the font has already been loaded it won't be loaded again.
        [[nodiscard]] std::shared_ptr<Font> loadFont(const std::string &name, const std::string &ttfFile,
                                                     FontStyle style, double size, double resolution);

        /// Retrieves an already loaded font or std::nullopt if the font has not been loaded yet.
        [[nodiscard]] std::optional<std::shared_ptr<Font>>
        getFont(const std::string &name, FontStyle style, float size, float resolution) const;

        /// Return a list of all available fonts by name
        [[nodiscard]] const std::vector<FontMeta> &getAll() const { return fontsMeta; }

    private:
        FT_Library freetype;
        std::vector<std::shared_ptr<Font>> loadedFonts;
        std::vector<FontMeta> fontsMeta;
    };

}

