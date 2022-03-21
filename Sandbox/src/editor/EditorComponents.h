#pragma once

#include <string>
#include <vector>
#include <optional>

namespace Editor {

    struct TextureMeta {
        const std::string slot;
        const std::string texture;

        TextureMeta(std::string slot, std::string texture) : slot(std::move(slot)), texture(std::move(texture)) {}

        ~TextureMeta() = default;
    };

    struct RenderComponentMeta {
        std::string meshName;
        std::string materialName;
        std::optional<std::vector<TextureMeta>> textures;
    };

    struct NativeScriptComponentMeta {
        std::string scriptName;
    };

}
