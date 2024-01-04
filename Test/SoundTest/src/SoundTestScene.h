#pragma once

#include "Engine/src/ChaosEngine.h"

class SoundTestScene : public ChaosEngine::Scene {
public:
    SoundTestScene() : Scene(glm::vec2{0, -10}), engine(nullptr), window(nullptr), gravity(0, -10) {}

    ~SoundTestScene() override = default;

    ChaosEngine::SceneConfiguration configure(ChaosEngine::Engine &engine) override;

    void load() override;

    void update(float deltaTime) override;

    void updateImGui() override;
private:
    void loadEntities();
private:
    ChaosEngine::Engine* engine;
    Window *window;
    glm::vec2 gravity;
    std::shared_ptr<ChaosEngine::AssetManager> assetManager;

    std::shared_ptr<Renderer::RenderMesh> quadROB = nullptr;
    Renderer::MaterialRef texturedMaterial  = Renderer::MaterialRef(nullptr);
    std::shared_ptr<ChaosEngine::AudioBuffer> backgroundAudioBuffer = nullptr;
    std::shared_ptr<ChaosEngine::AudioBuffer> stepsAudioBuffer = nullptr;

    ChaosEngine::Entity mainCamera;
    ChaosEngine::Entity audioTesterSurround;
    std::array<ChaosEngine::Entity, 4> spatialTesters;

};