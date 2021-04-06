#include "RenderingSystem.h"

#include "Engine.h"
#include "Engine/src/renderer/VulkanRenderer2D.h"

#include <iostream>

using namespace Renderer;

std::unique_ptr<GraphicsContext> RenderingSystem::Context = nullptr;
std::unique_ptr<RendererAPI> RenderingSystem::Renderer = nullptr;

RenderingSystem::RenderingSystem(Window &window) {
    Context = Renderer::GraphicsContext::Create(window, GraphicsAPI::Vulkan);
}

RenderingSystem::~RenderingSystem() {
    // Wait for renderer to finish
    if (Renderer != nullptr)
        Renderer->join();
    Renderer = nullptr;
    Context = nullptr;
}

void RenderingSystem::createRenderer(RendererType rendererType) {
    switch (rendererType) {
        case Renderer::RendererType::RENDERER2D :
            Renderer = VulkanRenderer2D::Create(*Context);
            break;
        default:
            assert("Unknown renderer" && false);
    }
    Renderer->setup();
}

void RenderingSystem::updateComponents(ECS &/*ecs*/) {
    Context->tickFrame();
}

void RenderingSystem::renderEntities(ECS &ecs) {
    assert("Renderer must be initialized" && Renderer != nullptr);
    auto view = ecs.getRegistry().view<const Transform, const RenderComponent>();
    auto cameras = ecs.getRegistry().view<const Transform, const CameraComponent>();

    Context->beginFrame();

    assert("There must be one active camera" && cameras.begin() != cameras.end());
    for (const auto&[entity, transform, camera] : cameras.each()) {
        Renderer->beginScene(transform.getModelMatrix(), camera);
        break;
    }
    for (const auto&[entity, transform, renderComp]: view.each()) {
        Renderer->draw(transform.getModelMatrix(), renderComp);
    }
    Renderer->endScene();

    // Push render commands to GPU
    Renderer->flush();
}