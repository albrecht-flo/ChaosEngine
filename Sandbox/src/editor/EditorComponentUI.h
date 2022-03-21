#pragma once

#include "Engine/src/core/Entity.h"
#include "Engine/src/core/Scene.h"
#include "EditorBaseAssets.h"
#include "EditorAssetSelector.h"

namespace Editor {

    class EditorComponentUI {
    public:
        explicit EditorComponentUI(const ChaosEngine::AssetManager &assetManager, const EditorBaseAssets &baseAssets)
                : assetManager(assetManager), editorAssets(baseAssets), assetSelector(assetManager) {}

        bool renderEntityComponentPanel(ChaosEngine::Entity &entity);

    private:
        void renderMetaComponentUI(ChaosEngine::Entity &entity);

        void renderTransformComponentUI(ChaosEngine::Entity &entity);

        void renderCameraComponentUI(ChaosEngine::Entity &entity);

        void renderRenderComponentUI(ChaosEngine::Entity &entity);

        void renderNativeScriptComponentUI(ChaosEngine::Entity &entity);

    private:

        void renderComponentPopupList(ChaosEngine::Entity &entity);

        void addComponentToEntity(ChaosEngine::Entity &entity, const std::string &component);

        void updateMaterialInstance(ChaosEngine::Entity &entity, glm::vec4 color, const RenderComponentMeta &rcMeta);

    private:
        const ChaosEngine::AssetManager &assetManager;
        const EditorBaseAssets &editorAssets;
        EditorAssetSelector assetSelector;
        glm::vec4 editTintColor = glm::vec4(1, 1, 1, 1);
        float dragSpeed = 1.0f;
        std::string componentMenuInput{};
        int selectedComponent = 0;
    public:
        static const std::array<std::string, 3> componentList;
    };
}
