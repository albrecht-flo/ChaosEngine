#pragma once

#include "Engine/src/core/Entity.h"
#include "Engine/src/core/Scene.h"
#include "EditorComponents.h"
#include "EditorBaseAssets.h"

namespace Editor {

    class EditorComponentUI {
    public:
        explicit EditorComponentUI(const AssetManager &assetManager, const EditorBaseAssets &baseAssets)
                : assetManager(assetManager), editorAssets(baseAssets) {}

        bool renderEntityComponentPanel(Entity &entity);

    private:
        void renderMetaComponentUI(Entity &entity);

        void renderTransformComponentUI(Entity &entity);

        void renderCameraComponentUI(Entity &entity);

        void renderRenderComponentUI(Entity &entity);

    private:

        void renderComponentPopupList(Entity &entity);

        void addComponentToEntity(Entity &entity, const std::string &component);

        void updateMaterialInstance(Entity &entity, glm::vec4 color, const RenderComponentMeta &rcMeta);

    private:
        const AssetManager &assetManager;
        const EditorBaseAssets &editorAssets;
        glm::vec4 editTintColor = glm::vec4(1, 1, 1, 1);
        float dragSpeed = 1.0f;
        std::string componentMenuInput{};
        int selectedComponent = 0;
    public:
        static const std::array<std::string, 2> componentList;
    };
}
