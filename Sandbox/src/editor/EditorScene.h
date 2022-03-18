#pragma once

#include "Engine/src/ChaosEngine.h"
#include "EditorBaseAssets.h"

namespace Editor {

    class EditorScene : public Scene {
    public:
        EditorScene() : Scene(), window(nullptr) {}

        ~EditorScene() override = default;

        SceneConfiguration configure(Window &window) override;

        void load() override;

        void update(float deltaTime) override;

        void updateImGui() override;

    private:

        void loadEntities();

        void addNewEntity();

        void imGuiMainMenu();

    private:
        Window *window;
        EditorBaseAssets baseAssets;
        Entity editorCamera;
    };

}

