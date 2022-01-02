#pragma once

#include "Engine/src/ChaosEngine.h"

class EmptyScene : public Scene {
public:
    EmptyScene() : Scene(), window(nullptr) {}

    ~EmptyScene() override = default;

    SceneConfiguration configure(Window &window) override;

    void load() override;

    void update(float deltaTime) override;

    void updateImGui() override;

private:
    Window *window;
};