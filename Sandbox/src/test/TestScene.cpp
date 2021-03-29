#include "TestScene.h"

#include "Engine/src/renderer/api/Material.h"
#include "Engine/src/core/Utils/Logger.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include "CustomImGui.h"

#include <iostream>

SceneConfiguration TestScene::configure(Window &pWindow) {
    window = &pWindow;
    return SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D
    };
}

void TestScene::load() {
    using namespace Renderer;
    // Load Materials
    coloredMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .fixedFunction = FixedFunctionConfiguration{.depthTest = true, .depthWrite = true},
            .vertexShader = "2DSprite",
            .fragmentShader = "2DStaticColoredSprite",
            .pushConstant = Material::StandardOpaquePushConstants,
            .set0 = Material::StandardOpaqueSet0,
            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
            .set1 = std::vector<ShaderBindings>(
                    {ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Fragment, .name="materialData",
                            .layout=std::vector<ShaderBindingLayout>(
                                    {
                                            ShaderBindingLayout{.type = ShaderValueType::Vec4, .name ="color"},
                                    })
                    }}),
            .set1ExpectedCount = 64,

    });
    texturedMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .fixedFunction = FixedFunctionConfiguration{.depthTest = true, .depthWrite = true},
            .vertexShader = "2DSprite",
            .fragmentShader = "2DStaticTexturedSprite",
            .pushConstant = Material::StandardOpaquePushConstants,
            .set0 = Material::StandardOpaqueSet0,
            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
            .set1 = std::vector<ShaderBindings>(
                    {ShaderBindings{.type = ShaderBindingType::TextureSampler, .stage=ShaderStage::Fragment, .name="diffuseTexture"},
                     ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Fragment, .name="materialData",
                             .layout=std::vector<ShaderBindingLayout>(
                                     {
                                             ShaderBindingLayout{.type = ShaderValueType::Vec4, .name ="color"},
                                     })
                     }
                    }),
            .set1ExpectedCount = 64,

    });

    fallbackTexture = Texture::Create("TestAtlas.jpg");
    loadEntities();

}

void TestScene::loadEntities() {
    cameraEnt = registry.createEntity();
    cameraEnt.setComponent<Transform>(Transform{glm::vec3(0, 0, -2), glm::vec3(), glm::vec3(1, 1, 1)});
    cameraEnt.setComponent<CameraComponent>(CameraComponent{
            .fieldOfView = 10.0f,
            .near = 0.1f,
            .far = 100.0f,
    });

    yellowQuad = registry.createEntity();
    yellowQuad.setComponent<Transform>(Transform{glm::vec3(), glm::vec3(), glm::vec3(1, 1, 1)});
    glm::vec4 yellowColor(1, 1, 0, 1);
    yellowQuad.setComponent<RenderComponent>(coloredMaterial.instantiate(&yellowColor, sizeof(yellowColor), {}));

    redQuad = registry.createEntity();
    redQuad.setComponent<Transform>(Transform{glm::vec3(2, 0, 0), glm::vec3(), glm::vec3(1, 1, 1)});
    glm::vec4 redColor(1, 0, 0, 1);
    redQuad.setComponent<RenderComponent>(coloredMaterial.instantiate(&redColor, sizeof(redColor), {}));

    texturedQuad = registry.createEntity();
    texturedQuad.setComponent<Transform>(Transform{glm::vec3(-4, 0, 0), glm::vec3(0, 0, 45), glm::vec3(1, 1, 1)});
    glm::vec4 whiteTintColor(1, 1, 1, 1);
    texturedQuad.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&whiteTintColor, sizeof(whiteTintColor), {fallbackTexture.get()}));
}

// Test data
static bool cameraControllerActive = false;
static bool itemEditActive = true;
static float dragSpeed = 1.0f;
static glm::vec3 origin(0, 0, -2.0f);
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

void TestScene::imGuiMainMenu() {
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Close", "Ctrl+Q")) {
            window->close();
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

}

static glm::vec4 editTintColor = glm::vec4(1, 1, 1, 1);

#include <renderer/vulkan/image/VulkanTexture.h> // TODO Remove here

void TestScene::updateImGui() {
    ImGui::NewFrame();
    CustomImGui::ImGuiEnableDocking([&]() { imGuiMainMenu(); });
    if (cameraControllerActive) {
        if (ImGui::Begin("CameraControl", &cameraControllerActive)) {
            ImGui::Text("Camera Controller");
            ImGui::SliderFloat("float", &cameraSpeed, 0.0f, 50.0f);
        }
        ImGui::End();
    }
    if (itemEditActive) {
        if (ImGui::Begin("ItemEdit", &itemEditActive)) {
            ImGui::Text("Edit Entity %X", static_cast<uint32_t>(texturedQuad));
            auto &tc = texturedQuad.get<Transform>();
            ImGui::DragFloat3("Position", &(tc.position.x), 0.25f * dragSpeed);
            ImGui::DragFloat3("Rotation", &(tc.rotation.x), 1.0f * dragSpeed);
            ImGui::DragFloat3("Scale", &(tc.scale.x), 0.25f * dragSpeed);
            ImGui::Separator();
            ImGui::ColorEdit4("Color", &(editTintColor.r));
            if (ImGui::Button("Apply")) {
                std::cout << "Apply new color" << std::endl;
                texturedQuad.setComponent<RenderComponent>(
                        texturedMaterial.instantiate(&editTintColor, sizeof(editTintColor), {fallbackTexture.get()}));
            }
        }
        ImGui::End();
    }

    CustomImGui::RenderLogWindow();

    // NEXT Rework ImGui Texture allocation (update); Resizing
    ImGui::Begin("Viewport");
//    auto size = ImGui::GetContentRegionMax();
//    LOG_DEBUG("Current Viewport size ({0} x {1})", size.x, size.y);
    // TODO Resize
    const auto &fb = RenderingSystem::GetCurrentRenderer().getFramebuffer();
    const auto &tex = dynamic_cast<const VulkanTexture &>(fb.getAttachmentTexture(Renderer::AttachmentType::Color, 0));
    // TODO Reuse (Currently this breaks after ~ 1000 allocations
    auto x = ImGui_ImplVulkan_AddTexture(tex.getSampler(), tex.getImageView(), tex.getImageLayout());
    // TODO(Idea): custom ImGui Function for
    ImGui::Image(x, ImVec2(fb.getWidth(), fb.getHeight()));
    ImGui::End();
}
