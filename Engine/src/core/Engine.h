#pragma once

#include <chrono>
#include "Engine/src/renderer/window/Window.h"
#include "Engine/src/renderer/api/GraphicsContext.h"
#include "RenderingSystem.h"
#include "scriptSystem/NativeScriptSystem.h"


namespace ChaosEngine {

    class Scene;

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
         *  TOBE: Parallelization > Apply component changes -> run systems
         */
        void run();

        static Window* getEngineWindow() { return engineWindow; }

    private:
        Window window;
        static Window* engineWindow;

        // Systems
        RenderingSystem renderingSys;
        NativeScriptSystem nativeScriptSystem;

        // Scene Data
        std::unique_ptr<Scene> scene;

        // FPS counter
        std::chrono::time_point<std::chrono::high_resolution_clock> deltaTimer;
        uint32_t frameCounter;
        float fpsDelta;
    };

}
