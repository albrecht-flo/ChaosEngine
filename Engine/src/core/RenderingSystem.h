#pragma once

#include <memory>
#include "Ecs.h"
#include "Components.h"


namespace Renderer {
    class Renderer {
    public:
        virtual void setup() = 0;

        virtual void beginScene(const glm::mat4 &viewMatrix, const CameraComponent &camera) = 0;

        virtual void draw(const glm::mat4 &viewMatrix, const RenderComponent &renderComponent) = 0;

        virtual void endScene() = 0;

        virtual void flush() = 0;

        virtual void join() = 0;
    };
}
class RenderingSystem {
public:
    RenderingSystem() = default;

    ~RenderingSystem();

    void updateComponents(ECS &ecs);

    void renderEntities(ECS &ecs);

    void setRenderer(std::unique_ptr<Renderer::Renderer> &&pRenderer);

private:
    std::unique_ptr<Renderer::Renderer> renderer = nullptr;
};
