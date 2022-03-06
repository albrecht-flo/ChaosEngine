#include "Engine.h"
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include "Scene.h"
#include "core/Utils/Logger.h"

Engine::Engine(std::unique_ptr<Scene> &&scene)
        : window(Window::Create("Chaos Engine", 1400, 800)),
          renderingSys(window, Renderer::GraphicsAPI::Vulkan),
          deltaTimer(std::chrono::high_resolution_clock::now()),
          frameCounter(0), fpsDelta(0) {
    assert("A Scene is required" && scene != nullptr);
    Logger::I("Engine", "Loading Scene");
    loadScene(std::move(scene));
}

void Engine::loadScene(std::unique_ptr<Scene> &&pScene) {
    scene = std::move(pScene);
    SceneConfiguration config = scene->configure(window);
    renderingSys.createRenderer(config.rendererType);

    scene->load();
}

void Engine::run() {
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

        // Update render system
        renderingSys.renderEntities(scene->ecs);


        // Update Scene // TODO: Process scripts instead ----------------------
        scene->update(deltaTime);

    }
    RenderingSystem::GetContext().waitIdle();
}

