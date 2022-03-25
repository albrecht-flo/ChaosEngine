#pragma once

#include <memory>
#include <vector>
#include <string>

namespace ChaosEngine {

    enum class FontStyle {Regular, Italic, Bold};

    class Font {

    };

    class FontManager {
    public:
        struct FontDefinition {
            std::string ttfFile;
            FontStyle style;
        };
    public:
        static std::shared_ptr<Font> Create(const std::vector<FontManager::FontDefinition>& fontParts);
    };

}
