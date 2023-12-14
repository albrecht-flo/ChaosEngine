#pragma once

#include "Engine/src/ChaosEngine.h"

class Box2DTestScene : public ChaosEngine::Scene {
public:
    Box2DTestScene() : Scene(), window(nullptr) {}

    ~Box2DTestScene() override = default;

    ChaosEngine::SceneConfiguration configure(ChaosEngine::Engine &engine) override;

    void load() override;

    void update(float deltaTime) override;

    void updateImGui() override;
private:
    void loadEntities();
private:
    Window *window;
    std::shared_ptr<ChaosEngine::AssetManager> assetManager;

    std::shared_ptr<Renderer::RenderMesh> quadROB = nullptr;
    Renderer::MaterialRef texturedMaterial  = Renderer::MaterialRef(nullptr);

    ChaosEngine::Entity mainCamera;
};