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
        bool debugRenderingEnabled = false;
    };

    /**
     *  Scenes are responsible for configuring the engine and managing entity lifetimes.
     */
    class Scene {
        friend class Engine;

    public:
        Scene() : physicsWorld(glm::vec2(0, 0)), ecs() {}

        Scene(const glm::vec2 &gravity) : physicsWorld(gravity), ecs() {}

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

        ECS &getECS() { return ecs; }

        PhysicsWorld &getPhysicsWorld() { return physicsWorld; }

    protected:
        PhysicsWorld physicsWorld;
        ECS ecs;
        //TOBE: Scene tree
    };

}
