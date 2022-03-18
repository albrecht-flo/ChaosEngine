#pragma once

#include "Engine/src/core/Entity.h"
#include "Engine/src/core/Scene.h"
#include "EditorBaseAssets.h"

namespace Editor {

    class EditorUI {
    private:
        struct EditorUIState {
            glm::vec4 editTintColor;
            float dragSpeed;
        };
    public:
        static bool renderEntityComponentPanel(Entity &entity, const EditorBaseAssets &assets);

    private:
        static void renderMetaComponentUI(Entity &entity);

        static void renderTransformComponentUI(Entity &entity);

        static void renderCameraComponentUI(Entity &entity);

        static void renderRenderComponentUI(Entity &entity, const EditorBaseAssets &assets);

    private:
        static EditorUIState state;
    };

}
