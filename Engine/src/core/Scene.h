#pragma once

#include "Ecs.h"
#include "Engine/src/renderer/api/RendererAPI.h"
#include "Engine/src/renderer/window/Window.h"

namespace ChaosEngine {

    /// Via this struct the scene is able to configure the engine runtime.
    struct SceneConfiguration {
        Renderer::RendererType rendererType; // TOBE: Abstraction layer
    };

    /**
     *  Scenes are responsible for configuring the engine and managing entity lifetimes.
     */
    class Scene {
        friend class Engine;

    public:
        Scene() = default;

        virtual ~Scene() = default;

        /// Returns the configuration of the engine for this scene.
        virtual SceneConfiguration configure(Engine &engine) = 0;

        /// Initializes the scene
        virtual void load() = 0;

        /// Update the scene
        virtual void update(float deltaTime) = 0;

        /// Here the scene can render ImGui interfaces
        virtual void updateImGui() = 0;

        Entity createEntity() { return ecs.addEntity(); }

    protected:
        ECS ecs;
        //TOBE: Scene tree
    };

}
