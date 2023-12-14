#pragma once

#include <memory>
#include "Engine/src/core/Ecs.h"
#include "Engine/src/core/Components.h"
#include "Engine/src/renderer/api/RendererAPI.h"
#include "Engine/src/renderer/api/GraphicsContext.h"
#include "UIRenderSubSystem.h"

namespace ChaosEngine {

    class RenderingSystem {
    public:
        RenderingSystem(Window &window, Renderer::GraphicsAPI api);

        ~RenderingSystem();

        /// Applies all changes that happened since last frame
        void updateComponents(ECS &ecs);

        /// Processes all entities to create the next frame
        void renderEntities(ECS &ecs);

        void createRenderer(Renderer::RendererType rendererType, bool renderSceneToOffscreenBuffer);

        static Renderer::GraphicsContext &GetContext() {
            assert("Context MUST not be empty!" && Context != nullptr);
            return *Context;
        }

        static Renderer::RendererAPI &GetCurrentRenderer() {
            assert("Renderer MUST not be empty!" && Renderer != nullptr);
            return *Renderer;
        }

    private:
        std::unique_ptr<UIRenderSubSystem> uiRenderSubSystem;
    private:
        static std::unique_ptr<Renderer::GraphicsContext> Context;
        static std::unique_ptr<Renderer::RendererAPI> Renderer;
    };

}