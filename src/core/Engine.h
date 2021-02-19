#pragma once

#include <chrono>
#include "src/renderer/window/Window.h"
#include "Scene.h"

class Engine {
public:
    explicit Engine(std::unique_ptr<Scene>&& scene);

    ~Engine() = default;

    void loadScene(std::unique_ptr<Scene>&& scene);

    void run();

private:
    Window window;
    std::unique_ptr<Scene> scene;

    // FPS counter
    std::chrono::time_point<std::chrono::steady_clock> deltaTimer;
    uint32_t frameCounter;
    float fpsDelta;

    // Per Scene configured objects
    std::unique_ptr<VulkanRenderer2D> renderer; // TODO: Abstraction layer
};



