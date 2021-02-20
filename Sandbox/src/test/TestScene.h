#pragma once

#include "Engine/src/ChaosEngine.h"

class TestScene : public Scene {
public:
    TestScene() : Scene(), window(nullptr), renderer(nullptr) {}

    ~TestScene() = default;

    SceneConfiguration configure(Window &window) override;

    void load() override;

    void update(float deltaTime) override;

    void updateImGui() override;

private:
    Window *window;
    VulkanRenderer2D *renderer;
    Entity cameraEnt;
    Entity whiteQuad;
    Entity redQuad;
    Entity greenQuad;
};



