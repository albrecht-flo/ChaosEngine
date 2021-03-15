#pragma once

#include <memory>
#include "Ecs.h"
#include "Components.h"
#include "Engine/src/renderer/window/Window.h"
#include "Engine/src/renderer/api/RendererAPI.h"
#include "Engine/src/renderer/api/GraphicsContext.h"

class RenderingSystem {
public:
    explicit RenderingSystem(Window &window);

    ~RenderingSystem();

    /// Applies all changes that happened since last frame
    void updateComponents(ECS &ecs);

    /// Processes all entities to create the next frame
    void renderEntities(ECS &ecs);

    void createRenderer(Renderer::RendererType rendererType);

    static Renderer::GraphicsContext &GetContext() { return *Context; }

    static Renderer::RendererAPI &GetCurrentRenderer() {
        assert("Context Must not be empty" && Renderer != nullptr);
        return *Renderer;
    }

private:
    std::unique_ptr<Renderer::GraphicsContext> context;
    std::unique_ptr<Renderer::RendererAPI> renderer;
    static Renderer::GraphicsContext *Context;
    static Renderer::RendererAPI *Renderer;
};
