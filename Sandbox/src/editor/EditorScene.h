#pragma once

#include "Engine/src/ChaosEngine.h"
#include "EditorBaseAssets.h"
#include "EditorComponentUI.h"

namespace Editor {

    class EditorScene : public Scene {
    public:
        EditorScene() : Scene(), window(nullptr), baseAssets(assetManager), editorUI(assetManager) {}

        ~EditorScene() override = default;

        SceneConfiguration configure(Window &window) override;

        void load() override;

        void update(float deltaTime) override;

        void updateImGui() override;

    private:

        void addNewEntity();

        void imGuiMainMenu();

    private:
        Window *window;
        EditorBaseAssets baseAssets;
        EditorComponentUI editorUI;
        Entity editorCamera;
    };

}

