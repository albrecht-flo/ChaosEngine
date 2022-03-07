#pragma once

#include "Engine/src/ChaosEngine.h"

class TestScene : public Scene {
public:
    TestScene() : Scene(), window(nullptr) {}

    ~TestScene() override = default;

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
    std::shared_ptr<Renderer::RenderMesh> quadROB;
    std::shared_ptr<Renderer::RenderMesh> hexROB;
    Renderer::MaterialRef debugMaterial = Renderer::MaterialRef(nullptr);
    Renderer::MaterialRef coloredMaterial = Renderer::MaterialRef(nullptr);
    Renderer::MaterialRef texturedMaterial = Renderer::MaterialRef(nullptr);
    std::unique_ptr<Renderer::Texture> fallbackTexture = nullptr;
    Entity editorCamera;
};



