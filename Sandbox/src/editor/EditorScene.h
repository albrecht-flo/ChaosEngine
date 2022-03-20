#pragma once

#include "Engine/src/ChaosEngine.h"
#include "EditorBaseAssets.h"
#include "EditorComponentUI.h"
#include "EditorAssetManager.h"

namespace Editor {

    class EditorScene : public ChaosEngine::Scene {
    public:
        EditorScene() : Scene(), window(nullptr), baseAssets(assetManager), editorUI(assetManager, baseAssets) {}

        ~EditorScene() override = default;

        ChaosEngine::SceneConfiguration configure(Window &window) override;

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
        EditorAssetManager editorAssetManager;
        ChaosEngine::Entity editorCamera;
    };

}

