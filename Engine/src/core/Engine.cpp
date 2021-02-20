#include "Engine.h"
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include "Components.h"

Engine::Engine(std::unique_ptr<Scene> &&scene)
        : window(Window::Create("Test Engine")),
          deltaTimer(std::chrono::high_resolution_clock::now()),
          frameCounter(0), fpsDelta(0) {
    assert("A Scene is required" && scene != nullptr);
    loadScene(std::move(scene));
}

void Engine::loadScene(std::unique_ptr<Scene> &&pScene) {
    scene = std::move(pScene);
    SceneConfiguration config = scene->configure(window);
    renderer = std::move(config.renderer);
    renderer->setup();

    scene->load();
}

static void renderEntities(entt::registry &registry, VulkanRenderer2D &renderer) {
    auto view = registry.view<const Transform, const RenderComponent>();
    auto camera = registry.view<const CameraComponent>();
    assert("There must be one active camera" && !camera.empty());

    renderer.beginScene(registry.get<CameraComponent>(camera.front()));
    for (const auto&[entity, transform, renderComp]: view.each()) {
        renderer.renderQuad(transform.getModelMatrix(), renderComp.color);
    }
    renderer.endScene();
}

void Engine::run() {

    while (!window.shouldClose()) {
        // FPS counter + delta time calculation -------------------------------
        frameCounter++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - deltaTimer).count();
        fpsDelta += deltaTime;
        deltaTimer = currentTime;
        if (fpsDelta >= 1.0f) {
            fpsDelta -= 1.0f;
            std::cout << "FPS: " << frameCounter << std::endl;
            frameCounter = 0;
        }
        // Get window events
        window.poolEvents();

        // TOBE: Apply all changes to components
        // --------------------------------------------------------------------
        // TOBE Syncpoint

        // Update all Systems -------------------------------------------------
        // Update ImGui
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        scene->updateImGui();
        ImGui::Render();

        // Update render system
        renderEntities(scene->registry.getRegistry(), *renderer);

        // Push render commands to GPU
        renderer->flush();

        // Update Scene // TODO: Process scrpits instead ----------------------
        scene->update(deltaTime);

    }

    // Wait for renderer to finish
    renderer->join();

}

