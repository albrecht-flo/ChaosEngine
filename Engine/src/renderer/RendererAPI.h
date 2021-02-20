#pragma once

#include "Engine/src/core/Components.h"

namespace Renderer {

    enum class RendererType {
        RENDERER2D
    };

    class RendererAPI {
    public:
        virtual void setup() = 0;

        virtual void beginScene(const glm::mat4 &viewMatrix, const CameraComponent &camera) = 0;

        virtual void draw(const glm::mat4 &viewMatrix, const RenderComponent &renderComponent) = 0;

        virtual void endScene() = 0;

        virtual void flush() = 0;

        virtual void join() = 0;
    };

}