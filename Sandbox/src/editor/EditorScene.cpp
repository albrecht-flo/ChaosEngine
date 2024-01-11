#include "EditorScene.h"

#include "Engine/src/core/assets/FontManager.h"

#include "Sandbox/src/common/CustomImGui.h"
#include "Sandbox/src/common/AssetView.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "DefaultProject.h"
#include "EditorComponentUI.h"
#include "scripts/EditorCameraScript.h"

using namespace Editor;
using namespace ChaosEngine;

ChaosEngine::SceneConfiguration EditorScene::configure(ChaosEngine::Engine &engine) {
    window = &engine.getEngineWindow();
    assetManager = engine.getAssetManager();
    return ChaosEngine::SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D,
            .renderSceneToOffscreenBuffer = true,
    };
}


void EditorScene::load() {
    using namespace Renderer;
    baseAssets = std::make_unique<EditorBaseAssets>(*assetManager);
    editorUI = std::make_unique<EditorComponentUI>(*assetManager, *baseAssets);
    editorAssetManager = std::make_unique<EditorAssetManager>();

    baseAssets->loadBaseMeshes();
    baseAssets->loadBaseMaterials();
    baseAssets->loadBaseTextures();
    baseAssets->loadBaseFonts();
    baseAssets->loadBaseScripts();

    editorCamera = createEntity();
    editorCamera.setComponent<Transform>(Transform{glm::vec3(0, 0, -2), glm::vec3(), glm::vec3(1, 1, 1)});
    editorCamera.setComponent<CameraComponent>(CameraComponent{
            .fieldOfView = 10.0f,
            .near = 0.1f,
            .far = 100.0f,
            .active = true,
            .mainCamera = true,
    });
    auto script = std::unique_ptr<ChaosEngine::NativeScript>(new EditorCameraScript(editorCamera));
    editorCamera.setComponent<NativeScriptComponent>(std::move(script), true);

    Editor::loadDefaultSceneEntities(*this, *baseAssets, *assetManager);

}

// Test data
static bool cameraControllerActive = false;
static bool viewportInFocus = false;
static bool itemEditActive = true;
static float cameraSpeed = 10.0f;

void EditorScene::update(float /*deltaTime*/) {
    // Close window controls
    if (window->isKeyDown(GLFW_KEY_ESCAPE) ||
        (window->isKeyDown(GLFW_KEY_Q) && window->isKeyDown(GLFW_KEY_LEFT_CONTROL))) { window->close(); }
    // ImGui Controls
    if (window->isKeyDown(GLFW_KEY_LEFT_ALT) && window->isKeyDown(GLFW_KEY_LEFT_SHIFT) &&
        window->isKeyDown(GLFW_KEY_C)) { cameraControllerActive = true; }
    if (window->isKeyDown(GLFW_KEY_LEFT_ALT) && window->isKeyDown(GLFW_KEY_LEFT_SHIFT) &&
        window->isKeyDown(GLFW_KEY_E)) { itemEditActive = true; }

    // Camera controls
    dynamic_cast<EditorCameraScript &>(*(editorCamera.get<NativeScriptComponent>().script)).setActive(viewportInFocus);


}

void EditorScene::addNewEntity() {
    auto entity = ecs.addEntity();
    entity.setComponent<Meta>(Meta{"New Entity"});
    entity.setComponent<Transform>(Transform{glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
}

void EditorScene::imGuiMainMenu() {
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Close", "Ctrl+Q")) {
            window->close();
        }
        if (ImGui::MenuItem("New Project")) {
            editorAssetManager->createProject();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("New Entity")) {
            addNewEntity();
        }
        ImGui::EndMenu();
    }
    editorAssetManager->renderAssetMenu();
    ImGui::EndMainMenuBar();

}

static bool showImGuiDebugger = false;
static bool showImGuiDemo = false;
static ECS::entity_t selectedSceneElement = ECS::null;
// Sanity check because the editor uses uint32_t to manage the entity selection
static_assert(std::is_same<uint32_t, std::underlying_type<entt::entity>::type>::value,
              "EnTT entity type does not match editor entity type!");

static void recursiveTreeDraw(entt::registry &registry, entt::entity entity) {
    using namespace CustomImGui;
    auto &sgc = registry.get<SceneGraphComponent>(entity);
    auto &meta = registry.get<Meta>(entity);
    auto id = ECS::to_integral(entity);
    if (sgc.firstChild != ECS::null) { // parent node
        if (CoreImGui::TreeNodeBegin(id, reinterpret_cast<uint32_t *>(&selectedSceneElement), meta.name.c_str())) {

            entt::entity cur = sgc.firstChild;
            SceneGraphComponent *curSGC = registry.try_get<SceneGraphComponent>(cur);
            do {
                recursiveTreeDraw(registry, cur);
                cur = curSGC->next;
                curSGC = registry.try_get<SceneGraphComponent>(cur);
            } while (cur != sgc.firstChild);
            CoreImGui::TreeNodeEnd();
        }
    } else {
        CoreImGui::TreeLeaf(id, reinterpret_cast<uint32_t *>(&selectedSceneElement), meta.name.c_str());
    }
};

void EditorScene::updateImGui() {
    using namespace CustomImGui;
    ImGui::NewFrame();
    CoreImGui::ImGuiEnableDocking([&]() { imGuiMainMenu(); });

    if (ImGui::Begin("Outline")) {
        const uint32_t createEntityButtonSize = 100;
        ImGui::SameLine((ImGui::GetWindowWidth() - createEntityButtonSize) / 2);
        if (ImGui::Button("Create Entity", ImVec2(createEntityButtonSize, 20))) {
            addNewEntity();
        }
        ImGui::Separator();
        if (ImGui::BeginPopupContextWindow("Outline-menu")) {
            if (ImGui::Selectable("Create Entity")) {
                addNewEntity();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        auto entityView = ecs.getRegistry().view<const Meta>(entt::exclude<SceneGraphComponent>);
        for (const auto &[entity, meta]: entityView.each()) {
            const auto id = ECS::to_integral(entity);
            CoreImGui::TreeLeaf(id, reinterpret_cast<uint32_t *>(&selectedSceneElement), meta.name.c_str());
        }


        auto sgEntityView = ecs.getRegistry().view<const Meta, const SceneGraphComponent>();
        for (const auto &[entity, meta, sgc]: sgEntityView.each()) {
            const auto id = ECS::to_integral(entity);
            if (sgc.firstChild != ECS::null && sgc.parent == ECS::null) { // Root parent node
                recursiveTreeDraw(ecs.getRegistry(), entity);
            } else if (sgc.parent == ECS::null) { // Orphan node
                std::string label = meta.name + " (Orphan)";
                CoreImGui::TreeLeaf(id, reinterpret_cast<uint32_t *>(&selectedSceneElement), label.c_str());
            }
            // Leaf nodes are in recursive tree draw
        }
    }
    ImGui::End();

    const auto &fb = ChaosEngine::RenderingSystem::GetCurrentRenderer().getFramebuffer();
    auto size = CoreImGui::RenderSceneViewport(fb, "Scene", &viewportInFocus);
    window->setGameWindowExtent(glm::ivec2{(int) size.first.x, (int) size.first.y},
                                glm::ivec2{(int) size.second.x, (int) size.second.y});

    ImGui::Begin("Info");
    ImGuiIO &io = ImGui::GetIO();
    // Basic info
    ImGui::Text("Frame: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Viewport Size: %f x %f", size.second.x - size.first.x, size.second.y - size.first.y);
    auto mouse = window->getAbsoluteMousePos();
    ImGui::Text("Mouse position Size: %f x %f", mouse.x - size.first.x, mouse.y - size.first.y);
    ImGui::Separator();
    if (ImGui::Button("Show ImGui Debugger")) {
        showImGuiDebugger = true;
    }
    if (ImGui::Button("Show ImGui Demo")) {
        showImGuiDemo = true;
    }
    ImGui::End();
    if (showImGuiDebugger) ImGui::ShowMetricsWindow(&showImGuiDebugger);
    if (showImGuiDemo) ImGui::ShowDemoWindow(&showImGuiDemo);

    if (cameraControllerActive) {
        if (ImGui::Begin("CameraControl", &cameraControllerActive)) {
            ImGui::Text("Camera Controller");
            ImGui::SliderFloat("float", &cameraSpeed, 0.0f, 50.0f);
            dynamic_cast<EditorCameraScript &>(*(editorCamera.get<NativeScriptComponent>().script))
                    .setSpeed(cameraSpeed);
        }
        ImGui::End();
    }

    if (itemEditActive) {
        if (ImGui::Begin("ItemEdit", &itemEditActive)) {
            if (selectedSceneElement != ECS::null) {
                auto entity = ecs.getEntity(selectedSceneElement);
                if (editorUI->renderEntityComponentPanel(entity)) {
                    ecs.removeEntity(entity);
                    selectedSceneElement = ECS::null;
                }
            } else {
                ImGui::Text("No Item selected");
            }
        }
        ImGui::End();
    }

    AssetView::renderAssetView();
    CoreImGui::RenderLogWindow(); // Render after AssetView to put initial focus here
}
