#pragma once

#include <memory>
#include <vector>
#include <optional>

#include <freetype/freetype.h>

#include "Font.h"

namespace ChaosEngine {

    /**
     * This class is a utility class for the asset manager because these resources may live longer than the entities
     * referencing them. While also being very slow to load due to font rendering.
     */
    class FontManager {
    public:
        FontManager();

        ~FontManager();

        /// Loads a font from disk, if the font has already been loaded it won't be loaded again.
        std::shared_ptr<Font> loadFont(const std::string &name, const std::string &ttfFile, FontStyle style,
                                       double size, double resolution);

        /// Retrieves an already loaded font or std::nullopt if the font has not been loaded yet.
        std::optional<std::shared_ptr<Font>>
        getFont(const std::string &name, FontStyle style, double size, double resolution);

    private:
        FT_Library freetype;
        std::vector<std::shared_ptr<Font>> loadedFonts;
    };

}

