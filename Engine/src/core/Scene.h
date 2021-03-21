#pragma once

#include "Ecs.h"
#include "Engine/src/renderer/api/RendererAPI.h"
#include "Engine/src/renderer/window/Window.h"

/// Via this struct the scene is able to configure the engine runtime.
struct SceneConfiguration {
    Renderer::RendererType rendererType; // TODO: Abstraction layer
};

/**
 *  Scenes are responsible for configuring the engine and managing entity lifetimes.
 */
class Scene {
    friend class Engine;

public:
    virtual ~Scene() = default;

    /// Returns the configuration of the engine for this scene.
    virtual SceneConfiguration configure(Window &window) = 0;

    /// Initializes the scene
    virtual void load() = 0;

    /// Update the scene
    virtual void update(float deltaTime) = 0;

    /// Here the scene can render ImGui interfaces
    virtual void updateImGui() = 0;

protected:
    ECS registry;
    //TODO: Scene tree
};