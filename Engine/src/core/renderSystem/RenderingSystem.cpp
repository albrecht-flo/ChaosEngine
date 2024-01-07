#include "RenderingSystem.h"

#include "Engine/src/core/Engine.h"
#include "renderer/VulkanRenderer2D.h"
#include "renderer/testRenderer/TestRenderer.h"

#include <iostream>

using namespace ChaosEngine;
using namespace Renderer;

std::unique_ptr<GraphicsContext> RenderingSystem::Context = nullptr;
std::unique_ptr<RendererAPI> RenderingSystem::Renderer = nullptr;

RenderingSystem::RenderingSystem(Window &window, GraphicsAPI api)
        : uiRenderSubSystem(std::make_unique<UIRenderSubSystem>()) {
    Context = Renderer::GraphicsContext::Create(window, api);
}

RenderingSystem::~RenderingSystem() {
    // Wait for renderer to finish
    if (Renderer != nullptr)
        Renderer->join();

    uiRenderSubSystem = nullptr;
    Renderer = nullptr;
    Context = nullptr;
}

void RenderingSystem::createRenderer(RendererType rendererType, bool renderSceneToOffscreenBuffer,
                                     bool enableDebugRendering) {
    LOG_DEBUG("[RenderingSystem] currentAPI {}", Context->currentAPI);
    if (Context->currentAPI == Renderer::GraphicsAPI::Vulkan) {
        switch (rendererType) {
            case Renderer::RendererType::RENDERER2D :
                Renderer = VulkanRenderer2D::Create(*Context, !renderSceneToOffscreenBuffer, enableDebugRendering);
                break;
            default:
                assert("Unknown renderer for 'Vulkan' GraphicsAPI" && false);
        }
    } else if (Context->currentAPI == Renderer::GraphicsAPI::Test) {
        switch (rendererType) {
            case Renderer::RendererType::RENDERER2D :
                Renderer = TestRenderer::TestRenderer::Create(*Context);
                break;
            default:
                assert("Unknown renderer for 'Test' GraphicsAPI" && false);
        }
    } else {
        assert("Unsupported API" && false);
    }
    Renderer->setup();
    uiRenderSubSystem->init(2048);
}

void RenderingSystem::updateComponents(ECS &/*ecs*/) {
    Context->tickFrame();
}

void RenderingSystem::renderEntities(ECS &ecs, const std::optional<std::shared_ptr<DebugRenderData>>& debugData) {
    assert("Renderer must be initialized" && Renderer != nullptr);
    auto view = ecs.getRegistry().view<const TransformComponent, const RenderComponent>();
    auto cameras = ecs.getRegistry().view<const TransformComponent, const CameraComponent>();

    Context->beginFrame();
    Renderer->beginFrame();

    bool rendered = false;
    glm::mat4 modelMat{};
    CameraComponent currentCamera{};
    for (const auto&[entity, transform, camera]: cameras.each()) {
        if (camera.active && !rendered) {
            modelMat = transform.local.getModelMatrix();
            currentCamera = camera;
            Renderer->beginScene(modelMat, currentCamera);
            rendered = true;
        } else if (camera.active && rendered) {
            LOG_WARN("Only one camera can be active at a time!");
        }
    }
    if (!rendered) {
        LOG_ERROR("There was no active camera so nothing was rendered!");
        assert("There was no active camera so nothing was rendered!");
    }

    for (const auto&[entity, transform, renderComp]: view.each()) {
        Renderer->draw(transform.local.getModelMatrix(), renderComp);
    }
    if(debugData)
        Renderer->drawSceneDebug(modelMat, currentCamera, **debugData);
    Renderer->endScene();

    uiRenderSubSystem->render(ecs, *Renderer);

    Renderer->endFrame();

    // Push render commands to GPU
    Renderer->flush();
}