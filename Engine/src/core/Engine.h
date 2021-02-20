#pragma once

#include <chrono>
#include "Engine/src/renderer/window/Window.h"
#include "Scene.h"
#include "RenderingSystem.h"

/**
 * This class is responsible for the orchestration of engine systems for processing entities and scenes.
 * It also holds and manages the engine context.
 */
class Engine {
public:
    /// Create an application window and initialize the engine context
    explicit Engine(std::unique_ptr<Scene> &&scene);

    ~Engine() = default;

    /**
     * Load scene configuration and apply the configuration to engine systems.
     * Run scene.load() to load all per scene resources.
     * @param scene
     */
    void loadScene(std::unique_ptr<Scene> &&scene);

    /**
     * Run main-loop: <br/>
     *  Update the engine systems.
     * TODO: Parallelization > Apply component changes -> run systems
     */
    void run();

private:
    Window window;
    std::unique_ptr<Scene> scene;

    // FPS counter
    std::chrono::time_point<std::chrono::steady_clock> deltaTimer;
    uint32_t frameCounter;
    float fpsDelta;

    // Per Scene configured objects
    RenderingSystem renderingSys{};
};



