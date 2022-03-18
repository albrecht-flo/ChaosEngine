#include "EditorScene.h"

#include "Sandbox/src/common/CustomImGui.h"
#include "Sandbox/src/common/AssetView.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "DefaultProject.h"
#include "EditorUI.h"

using namespace Editor;

SceneConfiguration EditorScene::configure(Window &pWindow) {
    window = &pWindow;
    return SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D
    };
}


void EditorScene::load() {
    using namespace Renderer;

    baseAssets.loadBaseMeshes();
    baseAssets.loadBaseMaterials();
    baseAssets.loadBaseTextures();

    editorCamera = createEntity();
    editorCamera.setComponent<Transform>(Transform{glm::vec3(0, 0, -2), glm::vec3(), glm::vec3(1, 1, 1)});
    editorCamera.setComponent<CameraComponent>(CameraComponent{
            .fieldOfView = 10.0f,
            .near = 0.1f,
            .far = 100.0f,
            .active = true,
            .mainCamera = true,
    });

    Editor::loadDefaultSceneEntities(*this, baseAssets);

}

// Test data
static bool cameraControllerActive = false;
static bool viewportInFocus = false;
static bool itemEditActive = true;
static glm::vec3 origin(0, 0, -2.0f);
static float cameraSpeed = 10.0f;

void EditorScene::update(float deltaTime) {
    // Close window controls
    if (window->isKeyDown(GLFW_KEY_ESCAPE) ||
        (window->isKeyDown(GLFW_KEY_Q) && window->isKeyDown(GLFW_KEY_LEFT_CONTROL))) { window->close(); }
    // ImGui Controls
    if (window->isKeyDown(GLFW_KEY_LEFT_ALT) && window->isKeyDown(GLFW_KEY_LEFT_SHIFT) &&
        window->isKeyDown(GLFW_KEY_C)) { cameraControllerActive = true; }
    if (window->isKeyDown(GLFW_KEY_LEFT_ALT) && window->isKeyDown(GLFW_KEY_LEFT_SHIFT) &&
        window->isKeyDown(GLFW_KEY_E)) { itemEditActive = true; }

    // Camera controls
    if (viewportInFocus) {
        if (window->isKeyDown(GLFW_KEY_W)) { origin.y -= cameraSpeed * deltaTime; }
        if (window->isKeyDown(GLFW_KEY_S)) { origin.y += cameraSpeed * deltaTime; }
        if (window->isKeyDown(GLFW_KEY_A)) { origin.x += cameraSpeed * deltaTime; }
        if (window->isKeyDown(GLFW_KEY_D)) { origin.x -= cameraSpeed * deltaTime; }
        if (window->isKeyDown(GLFW_KEY_KP_ADD)) {
            editorCamera.get<CameraComponent>().fieldOfView -= 5 * deltaTime;
        } // TODO: Remove delta time after input refactor
        if (window->isKeyDown(GLFW_KEY_KP_SUBTRACT)) {
            editorCamera.get<CameraComponent>().fieldOfView += 5 * deltaTime;
        } // TODO: Remove delta time after input refactor

        editorCamera.get<Transform>().position = origin;
    }

}

void EditorScene::addNewEntity() {
    auto entity = ecs.addEntity();
    entity.setComponent<Meta>(Meta{"New Entity"});
    entity.setComponent<Transform>(Transform{glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
    glm::vec4 whiteTintColor(1, 1, 1, 1);
    entity.setComponent<RenderComponent>(
            baseAssets.getTexturedMaterial().instantiate(&whiteTintColor, sizeof(whiteTintColor),
                                                         {&baseAssets.getFallbackTexture()}),
            baseAssets.getQuadMesh());
}

void EditorScene::imGuiMainMenu() {
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Close", "Ctrl+Q")) {
            window->close();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("New Entity")) {
            addNewEntity();
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

}

static bool showImGuiDebugger = false;
static bool showImGuiDemo = false;
static ECS::entity_t selectedSceneElement = ECS::null;
// Sanity check because the editor uses uint32_t to manage the entity selection
static_assert(std::is_same<uint32_t, std::underlying_type<entt::entity>::type>::value,
              "EnTT entity type does not match editor entity type!");

void EditorScene::updateImGui() {
    using namespace CustomImGui;
    ImGui::NewFrame();
    CoreImGui::ImGuiEnableDocking([&]() { imGuiMainMenu(); });

    CoreImGui::RenderLogWindow();

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

        auto entityView = ecs.getRegistry().view<Meta>();
        entityView.each([](auto entity, Meta &meta) {
            const auto id = ECS::to_integral(entity);
            CoreImGui::TreeLeaf(id, reinterpret_cast<uint32_t *>(&selectedSceneElement), meta.name.c_str());
        });
    }
    ImGui::End();

    const auto &fb = RenderingSystem::GetCurrentRenderer().getFramebuffer();
    auto size = CoreImGui::RenderSceneViewport(fb, "Scene", &viewportInFocus);

    ImGui::Begin("Info");
    ImGuiIO &io = ImGui::GetIO();
    // Basic info
    ImGui::Text("Frame: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Viewport Size: %f x %f", size.x, size.y);
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
        }
        ImGui::End();
    }

    if (itemEditActive) {
        if (ImGui::Begin("ItemEdit", &itemEditActive)) {
            if (selectedSceneElement != ECS::null) {
                auto entity = ecs.getEntity(selectedSceneElement);
                if (editorUI.renderEntityComponentPanel(entity)) {
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
}
