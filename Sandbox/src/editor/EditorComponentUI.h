#pragma once

#include "Engine/src/core/Entity.h"
#include "Engine/src/core/Scene.h"
#include "EditorComponents.h"

namespace Editor {

    class EditorComponentUI {
    public:
        explicit EditorComponentUI(const AssetManager &assetManager) : assetManager(assetManager) {}

        bool renderEntityComponentPanel(Entity &entity);

    private:
        void renderMetaComponentUI(Entity &entity);

        void renderTransformComponentUI(Entity &entity);

        void renderCameraComponentUI(Entity &entity);

        void renderRenderComponentUI(Entity &entity);

    private:

        void updateMaterialInstance(Entity &entity, glm::vec4 color, const RenderComponentMeta &rcMeta);

    private:
        const AssetManager &assetManager;
        glm::vec4 editTintColor = glm::vec4(1, 1, 1, 1);
        float dragSpeed = 1.0f;
    };
}
