#include "RenderingSystem.h"
#include "Engine.h"

#include <iostream>
#include "Engine/src/renderer/VulkanRenderer2D.h"

using namespace Renderer;

GraphicsContext *RenderingSystem::Context = nullptr;
RendererAPI *RenderingSystem::Renderer = nullptr;

RenderingSystem::RenderingSystem(Window &window) {
    context = Renderer::GraphicsContext::Create(window, GraphicsAPI::Vulkan);
    Context = context.get();
}

RenderingSystem::~RenderingSystem() {
    // Wait for renderer to finish
    if (renderer != nullptr)
        renderer->join();
    Context = nullptr;
    Renderer = nullptr;
}

void RenderingSystem::createRenderer(RendererType rendererType) {
    switch (rendererType) {
        case Renderer::RendererType::RENDERER2D :
            renderer = VulkanRenderer2D::Create(*context);
            Renderer = renderer.get();
            break;
        default:
            assert("Unknown renderer" && false);
    }
    renderer->setup();
}

void RenderingSystem::updateComponents(ECS &ecs) {
    context->tickFrame();
}

void RenderingSystem::renderEntities(ECS &ecs) {
    assert("Renderer must be initialized" && renderer != nullptr);
    auto view = ecs.getRegistry().view<const Transform, const RenderComponent>();
    auto cameras = ecs.getRegistry().view<const Transform, const CameraComponent>();

    context->beginFrame();

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