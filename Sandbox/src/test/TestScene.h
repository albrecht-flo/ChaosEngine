#pragma once

#include "Engine/src/ChaosEngine.h"

class TestScene : public Scene {
public:
    TestScene() : Scene(), window(nullptr) {}

    ~TestScene() = default;

    SceneConfiguration configure(Window &window) override;

    void load() override;

    void update(float deltaTime) override;

    void updateImGui() override;

private:
    Window *window;
    std::unique_ptr<Renderer::Material> coloredMaterial = nullptr;
    std::unique_ptr<Renderer::Material> texturedMaterial = nullptr;
    std::unique_ptr<Renderer::Texture> fallbackTexture = nullptr;
    Entity cameraEnt;
    Entity yellowQuad;
    Entity redQuad;
    Entity texturedQuad;

    void loadEntities();
};



