#pragma once

#include "Engine/src/ChaosEngine.h"
#include "EditorBaseAssets.h"
#include "EditorComponentUI.h"
#include "EditorAssetManager.h"

namespace Editor {

    class EditorScene : public ChaosEngine::Scene {
    public:
        EditorScene() : Scene(), window(nullptr),
                        baseAssets(nullptr), editorUI(nullptr), editorAssetManager(nullptr) {}

        ~EditorScene() override = default;

        ChaosEngine::SceneConfiguration configure(ChaosEngine::Engine &engine) override;

        void load() override;

        void update(float deltaTime) override;

        void updateImGui() override;

    private:

        void addNewEntity();

        void imGuiMainMenu();

    private:
        Window *window;
        std::shared_ptr<ChaosEngine::AssetManager> assetManager;

        std::unique_ptr<EditorBaseAssets> baseAssets;
        std::unique_ptr<EditorComponentUI> editorUI;
        std::unique_ptr<EditorAssetManager> editorAssetManager;

        ChaosEngine::Entity editorCamera;
    };

}

