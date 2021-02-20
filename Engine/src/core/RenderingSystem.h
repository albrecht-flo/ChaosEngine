#pragma once

#include <memory>
#include "Ecs.h"
#include "Components.h"
#include "Engine/src/renderer/RendererAPI.h"

class RenderingSystem {
public:
    RenderingSystem() = default;

    ~RenderingSystem();

    void updateComponents(ECS &ecs);

    void renderEntities(ECS &ecs);

    void setRenderer(std::unique_ptr<Renderer::RendererAPI> &&pRenderer);

private:
    std::unique_ptr<Renderer::RendererAPI> renderer = nullptr;
};
