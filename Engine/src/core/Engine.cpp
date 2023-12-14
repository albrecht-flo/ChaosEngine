#include "Engine.h"
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include "Scene.h"
#include "core/utils/Logger.h"

using namespace ChaosEngine;

Engine *ChaosEngine::Engine::s_engineInstance = nullptr;

Engine::Engine()
        : window(Window::Create("Chaos Engine", 1400, 800)),
          renderingSys(window, Renderer::GraphicsAPI::Vulkan),
          uiSystem(renderingSys, window),
          nativeScriptSystem(),
          physicsSystem(),
          assetManager(std::make_shared<AssetManager>()),
          scene(nullptr),
          deltaTimer(std::chrono::high_resolution_clock::now()),
          frameCounter(0), fpsDelta(0) {
    if (s_engineInstance != nullptr) {
        throw std::runtime_error("There can only be one running Engine instance!");
    }
    s_engineInstance = this;
    Logger::I("Engine", "Loading Scene");
}

void Engine::loadScene(std::unique_ptr<Scene> &&pScene) {
    assert("A Scene is required." && pScene != nullptr);
    scene = std::move(pScene);
    SceneConfiguration config = scene->configure(*this);
    renderingSys.createRenderer(config.rendererType, config.renderSceneToOffscreenBuffer);
    physicsSystem.init(config);

    scene->load();

    nativeScriptSystem.init(scene->ecs);
    uiSystem.init(scene->ecs);
}

void Engine::run() {
    assert("A Scene is required for the engine to run!" && scene != nullptr);
    while (!window.shouldClose()) {
        Logger::Tick();
        // FPS counter + delta time calculation -------------------------------
        frameCounter++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - deltaTimer).count();
        fpsDelta += deltaTime;
        deltaTimer = currentTime;
        if (fpsDelta >= 1.0f) {
            fpsDelta -= 1.0f;
            LOG_INFO("FPS: {0}", frameCounter);
            frameCounter = 0;
        }
        // Get window events
        window.poolEvents();

        // TOBE: Apply all changes to components
        renderingSys.updateComponents(scene->ecs);
        // --------------------------------------------------------------------
        // TOBE Syncpoint

        // Update all Systems -------------------------------------------------
        // Update ImGui
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        scene->updateImGui();
        ImGui::Render();

        // Handle user input on ui elements
        uiSystem.update(scene->ecs);
        // Update render system
        renderingSys.renderEntities(scene->ecs);

        // Run all script updates
        nativeScriptSystem.update(scene->ecs, deltaTime);
        // Tick rendering system
        physicsSystem.update(scene->ecs, deltaTime);

        // Update Scene
        scene->update(deltaTime);

    }
    RenderingSystem::GetContext().waitIdle();
}

