#include "RenderingSystem.h"

#include <iostream>

RenderingSystem::~RenderingSystem() {
    // Wait for renderer to finish
    renderer->join();
}

void RenderingSystem::setRenderer(std::unique_ptr<Renderer::RendererAPI> &&pRenderer) {
    renderer = std::move(pRenderer);
    renderer->setup();
}

void RenderingSystem::updateComponents(ECS &ecs) {

}

void RenderingSystem::renderEntities(ECS &ecs) {
    auto view = ecs.getRegistry().view<const Transform, const RenderComponent>();
    auto cameras = ecs.getRegistry().view<const Transform, const CameraComponent>();

    assert("There must be one active camera" && cameras.begin() != cameras.end());
    for (const auto&[entity, transform, camera] : cameras.each()) {
        renderer->beginScene(transform.getModelMatrix(), camera);
        break;
    }
    for (const auto&[entity, transform, renderComp]: view.each()) {
        renderer->draw(transform.getModelMatrix(), renderComp);
    }
    renderer->endScene();

    // Push render commands to GPU
    renderer->flush();
}