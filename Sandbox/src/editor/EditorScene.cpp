#include "EditorScene.h"

#include "Engine/src/core/uiSystem/FontManager.h"

#include "Sandbox/src/common/CustomImGui.h"
#include "Sandbox/src/common/AssetView.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "DefaultProject.h"
#include "EditorComponentUI.h"
#include "Sandbox/src/editor/scripts/EditorCameraScript.h"

using namespace Editor;
using namespace ChaosEngine;

ChaosEngine::SceneConfiguration EditorScene::configure(ChaosEngine::Engine &engine) {
    window = &engine.getEngineWindow();
    assetManager = engine.getAssetManager();
    return ChaosEngine::SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D
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

    Editor::loadDefaultSceneEntities(*this, *baseAssets);

    auto openSansFont = FontManager::Create("OpenSauceSans", {
            {"fonts/OpenSauceSans-Regular.ttf", FontStyle::Regular},
            {"fonts/OpenSauceSans-Italic.ttf",  FontStyle::Italic},
            {"fonts/OpenSauceSans-Bold.ttf",    FontStyle::Bold}}
    );
//    assetManager.registerFont("OpenSauceSans", openSansFont);

    auto textTester1 = createEntity();
    textTester1.setComponent<Meta>("Text Tester unknown char");
    textTester1.setComponent<Transform>(Transform{glm::vec3{16, 64, -1}, glm::vec3(), glm::vec3(1, 1, 1)});
    textTester1.setComponent<UITextComponent>(UITextComponent{
            .font = openSansFont, // assetManager.getFont("OpenSauceSans"),
            .style = FontStyle::Regular,
            .textColor = glm::vec4(1, 1, 1, 1),
            .text = "Tise\3 äöü",
    });

    auto textTester2 = createEntity();
    textTester2.setComponent<Meta>("Text Tester Fox");
    textTester2.setComponent<Transform>(Transform{glm::vec3{16, 128, -1}, glm::vec3(), glm::vec3(1, 1, 1)});
    textTester2.setComponent<UITextComponent>(UITextComponent{
            .font = openSansFont, // assetManager.getFont("OpenSauceSans"),
            .style = FontStyle::Regular,
            .textColor = glm::vec4(1, 1, 1, 1),
            .text = "the quick brown fox jumps over the lazy dog! 0123456789",
    });

    auto textTester3 = createEntity();
    textTester3.setComponent<Meta>("Text Tester Multiline");
    textTester3.setComponent<Transform>(Transform{glm::vec3{16, 256, -1}, glm::vec3(), glm::vec3(1, 1, 1)});
    textTester3.setComponent<UITextComponent>(UITextComponent{
            .font = openSansFont, // assetManager.getFont("OpenSauceSans"),
            .style = FontStyle::Regular,
            .textColor = glm::vec4(0.3f, 0, 0.3f, 1),
            .text = "This is some Text with,\nmore than 1 lines :)\nAnd Special Character xD\n!@#$%^&*()-_=+[]{}'\":;,.<>/?",
    });

}

// Test data
static bool cameraControllerActive = false;
static bool viewportInFocus = false;
static bool itemEditActive = true;
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
    dynamic_cast<EditorCameraScript &>(*(editorCamera.get<NativeScriptComponent>().script)).setActive(viewportInFocus);


}

void EditorScene::addNewEntity() {
    auto entity = ecs.addEntity();
    entity.setComponent<Meta>(Meta{"New Entity"});
    entity.setComponent<Transform>(Transform{glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
    glm::vec4 whiteTintColor(1, 1, 1, 1);
    entity.setComponent<RenderComponent>(
            baseAssets->getTexturedMaterial().instantiate(&whiteTintColor, sizeof(whiteTintColor),
                                                          {&(baseAssets->getFallbackTexture())}),
            baseAssets->getQuadMesh());
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

    const auto &fb = ChaosEngine::RenderingSystem::GetCurrentRenderer().getFramebuffer();
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
}
