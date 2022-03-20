#pragma once

#include <memory>
#include "Ecs.h"
#include "Components.h"
#include "renderer/api/RendererAPI.h"
#include "renderer/api/GraphicsContext.h"

namespace ChaosEngine {

    class RenderingSystem {
    public:
        explicit RenderingSystem(Window &window, Renderer::GraphicsAPI api);

        ~RenderingSystem();

        /// Applies all changes that happened since last frame
        void updateComponents(ECS &ecs);

        /// Processes all entities to create the next frame
        void renderEntities(ECS &ecs);

        void createRenderer(Renderer::RendererType rendererType);

        static Renderer::GraphicsContext &GetContext() {
            assert("Context MUST not be empty!" && Context != nullptr);
            return *Context;
        }

        static Renderer::RendererAPI &GetCurrentRenderer() {
            assert("Renderer MUST not be empty!" && Renderer != nullptr);
            return *Renderer;
        }

    private:
        static std::unique_ptr<Renderer::GraphicsContext> Context;
        static std::unique_ptr<Renderer::RendererAPI> Renderer;
    };

}