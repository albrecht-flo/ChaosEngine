#pragma once

#include "Ecs.h"
#include "physicsSystem/PhysicsWorld.h"
#include "Engine/src/renderer/api/RendererAPI.h"
#include "Engine/src/renderer/window/Window.h"

namespace ChaosEngine {
    class Engine;

    /// Via this struct the scene is able to configure the engine runtime.
    struct SceneConfiguration {
        Renderer::RendererType rendererType; // TOBE: Abstraction layer
        bool renderSceneToOffscreenBuffer;
        glm::vec2 gravity = glm::vec2(0, 0);
        bool debugRenderingEnabled = false;
    };

    /**
     *  Scenes are responsible for configuring the engine and managing entity lifetimes.
     */
    class Scene {
        friend class Engine;

    public:
        Scene() = default;

        virtual ~Scene() = default;

        Scene(const Scene &o) = delete;

        Scene &operator=(const Scene &o) = delete;

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
        PhysicsWorld physicsWorld;
        ECS ecs;
        //TOBE: Scene tree
    };

}
