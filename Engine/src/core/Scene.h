#pragma once

#include "Ecs.h"

#include "Engine/src/renderer/VulkanRenderer2D.h"

struct SceneConfiguration {
    std::unique_ptr<VulkanRenderer2D> renderer; // TODO: Abstraction layer
};

class Scene {
    friend class Engine;

public:
    virtual SceneConfiguration configure(Window &window) = 0;

    virtual void load() = 0;

    virtual void update(float deltaTime) = 0;

    virtual void updateImGui() = 0;

protected:
    ECS registry;
    //TODO: Scene tree
};