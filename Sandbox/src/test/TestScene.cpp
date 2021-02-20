#include "TestScene.h"

SceneConfiguration TestScene::configure(Window &pWindow) {
    window = &pWindow;
    auto configRenderer = VulkanRenderer2D::Create(pWindow);
    renderer = configRenderer.get();
    return SceneConfiguration{
            std::move(configRenderer)
    };
}

void TestScene::load() {
    cameraEnt = registry.createEntity();
    cameraEnt.setComponent<Transform>(Transform{glm::vec3(0, 0, -2), glm::vec3(), glm::vec3(1, 1, 1)});
    cameraEnt.setComponent<CameraComponent>(CameraComponent{
            .fieldOfView = 10.0f,
            .near = 0.1f,
            .far = 100.0f,
    });

    whiteQuad = registry.createEntity();
    whiteQuad.setComponent<Transform>(Transform{glm::vec3(), glm::vec3(), glm::vec3(1, 1, 1)});
    whiteQuad.setComponent<RenderComponent>(glm::vec4(1));

    redQuad = registry.createEntity();
    redQuad.setComponent<Transform>(Transform{glm::vec3(2, 0, 0), glm::vec3(), glm::vec3(1, 1, 1)});
    redQuad.setComponent<RenderComponent>(glm::vec4(1, 0, 0, 1));

    greenQuad = registry.createEntity();
    greenQuad.setComponent<Transform>(Transform{glm::vec3(-4, 0, 0), glm::vec3(0, 0, 45), glm::vec3(1, 1, 1)});
    greenQuad.setComponent<RenderComponent>(glm::vec4(0, 1, 0, 1));

    renderer->updatePostProcessingConfiguration(PostProcessingPass::PostProcessingConfiguration{
            .camera = cameraEnt.get<CameraComponent>()
    });
}

// Test data
static bool cameraControllerActive = false;
static bool itemEditActive = true;
static float dragSpeed = 1.0f;
static glm::vec3 origin = glm::vec3(0, 0, -2.0f);
static float cameraSpeed = 10.0f;

void TestScene::update(float deltaTime) {
    // Close window controls
    if (window->isKeyDown(GLFW_KEY_ESCAPE) ||
        (window->isKeyDown(GLFW_KEY_Q) && window->isKeyDown(GLFW_KEY_LEFT_CONTROL))) { window->close(); }
    // ImGui Controls
    if (window->isKeyDown(GLFW_KEY_LEFT_ALT) && window->isKeyDown(GLFW_KEY_LEFT_SHIFT) &&
        window->isKeyDown(GLFW_KEY_C)) { cameraControllerActive = true; }
    if (window->isKeyDown(GLFW_KEY_LEFT_ALT) && window->isKeyDown(GLFW_KEY_LEFT_SHIFT) &&
        window->isKeyDown(GLFW_KEY_E)) { itemEditActive = true; }
    if (window->isKeyDown(GLFW_KEY_LEFT_CONTROL)) { dragSpeed = 0.125f; } else { dragSpeed = 1.0f; }

    // Camera controls
    if (window->isKeyDown(GLFW_KEY_W)) { origin.y -= cameraSpeed * deltaTime; }
    if (window->isKeyDown(GLFW_KEY_S)) { origin.y += cameraSpeed * deltaTime; }
    if (window->isKeyDown(GLFW_KEY_A)) { origin.x += cameraSpeed * deltaTime; }
    if (window->isKeyDown(GLFW_KEY_D)) { origin.x -= cameraSpeed * deltaTime; }
    if (window->isKeyDown(GLFW_KEY_KP_ADD)) {
        cameraEnt.get<CameraComponent>().fieldOfView -= 5 * deltaTime;
    } // TODO: Remove delta time after input refactor
    if (window->isKeyDown(GLFW_KEY_KP_SUBTRACT)) {
        cameraEnt.get<CameraComponent>().fieldOfView += 5 * deltaTime;
    } // TODO: Remove delta time after input refactor

    cameraEnt.get<Transform>().position = origin;

}

void TestScene::updateImGui() {
    ImGui::NewFrame();
    if (cameraControllerActive) {
        if (ImGui::Begin("CameraControl", &cameraControllerActive)) {
            ImGui::Text("Camera Controller");
            ImGui::SliderFloat("float", &cameraSpeed, 0.0f, 50.0f);
        }
        ImGui::End();
    }
    if (itemEditActive) {
        if (ImGui::Begin("ItemEdit", &itemEditActive)) {
            ImGui::Text("Edit Entity %x", greenQuad);
            auto &tc = greenQuad.get<Transform>();
            ImGui::DragFloat3("Position", &(tc.position.x), 0.25f * dragSpeed);
            ImGui::DragFloat3("Rotation", &(tc.rotation.x), 1.0f * dragSpeed);
            ImGui::DragFloat3("Scale", &(tc.scale.x), 0.25f * dragSpeed);
            ImGui::Separator();
            auto &rc = greenQuad.get<RenderComponent>();
            ImGui::ColorEdit4("Color", &(rc.color.r));
        }
        ImGui::End();
    }
}
