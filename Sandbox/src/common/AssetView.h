#pragma once

#include <cstdint>

namespace CustomImGui {
    class AssetView {
    private:
        struct AssetViewState {
            uint32_t selected = 0;
        };
    public:
        static bool renderAssetView();

    private:
        static AssetViewState state;
    };

}

