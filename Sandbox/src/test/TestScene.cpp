#include "TestScene.h"

#include "CustomImGui.h"

#include <imgui.h>


SceneConfiguration TestScene::configure(Window &pWindow) {
    window = &pWindow;
    return SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D
    };
}


void TestScene::load() {
    using namespace Renderer;
    // Load meshes
    LOG_INFO("Creating quad buffers");
    auto quadAsset = ModelLoader::getQuad();
    auto vertexBuffer = Buffer::Create(quadAsset.vertices.data(), quadAsset.vertices.size() * sizeof(Vertex),
                                       BufferType::Vertex);
    auto indexBuffer = Buffer::Create(quadAsset.indices.data(), quadAsset.indices.size() * sizeof(uint32_t),
                                      BufferType::Index);
    quadROB = RenderMesh::Create(std::move(vertexBuffer), std::move(indexBuffer), quadAsset.indices.size());

    LOG_INFO("Creating hex buffers");
    auto hexAsset = ModelLoader::getHexagon();
    auto hexVertexBuffer = Buffer::Create(hexAsset.vertices.data(), hexAsset.vertices.size() * sizeof(Vertex),
                                          BufferType::Vertex);
    auto hexIndexBuffer = Buffer::Create(hexAsset.indices.data(), hexAsset.indices.size() * sizeof(uint32_t),
                                         BufferType::Index);
    hexROB = RenderMesh::Create(std::move(hexVertexBuffer), std::move(hexIndexBuffer), hexAsset.indices.size());

    LOG_INFO("Creating materials");
    // Load Materials
//    debugMaterial = Material::Create(MaterialCreateInfo{
//            .stage = ShaderPassStage::Opaque,
//            .fixedFunction = FixedFunctionConfiguration{.topology = Topology::TriangleList, .polygonMode = PolygonMode::Line,
//                    .depthTest = true, .depthWrite = true},
//            .vertexShader = "2DDebug",
//            .fragmentShader = "2DStaticColoredSprite",
//            .pushConstant = Material::StandardOpaquePushConstants,
//            .set0 = Material::StandardOpaqueSet0,
//            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
//            .set1 = std::vector<ShaderBindings>(
//                    {ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Fragment, .name="materialData",
//                            .layout=std::vector<ShaderBindingLayout>(
//                                    {
//                                            ShaderBindingLayout{.type = ShaderValueType::Vec4, .name ="color"},
//                                    })
//                    }}),
//            .set1ExpectedCount = 64,
//            .name="DebugWireFrame",
//    });

    coloredMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .fixedFunction = FixedFunctionConfiguration{.depthTest = true, .depthWrite = true},
            .vertexShader = "2DSprite",
            .fragmentShader = "2DStaticColoredSprite",
            .pushConstant = std::make_optional(Material::StandardOpaquePushConstants),
            .set0 = std::make_optional(Material::StandardOpaqueSet0),
            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
            .set1 = std::make_optional(std::vector<ShaderBindings>(
                    {ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Fragment, .name="materialData",
                            .layout=std::make_optional(std::vector<ShaderBindingLayout>(
                                    {
                                            ShaderBindingLayout{.type = ShaderValueType::Vec4, .name ="color"},
                                    }))
                    }})),
            .set1ExpectedCount = 64,
            .name="ColoredSprite",
    });
    texturedMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .fixedFunction = FixedFunctionConfiguration{.depthTest = true, .depthWrite = true},
            .vertexShader = "2DSprite",
            .fragmentShader = "2DStaticTexturedSprite",
            .pushConstant = std::make_optional(Material::StandardOpaquePushConstants),
            .set0 = std::make_optional(Material::StandardOpaqueSet0),
            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
            .set1 = std::make_optional(std::vector<ShaderBindings>(
                    {ShaderBindings{.type = ShaderBindingType::TextureSampler, .stage=ShaderStage::Fragment, .name="diffuseTexture"},
                     ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Fragment, .name="materialData",
                             .layout=std::make_optional(std::vector<ShaderBindingLayout>(
                                     {
                                             ShaderBindingLayout{.type = ShaderValueType::Vec4, .name ="color"},
                                     }))
                     }
                    })),
            .set1ExpectedCount = 64,
            .name="TexturedSprite",
    });

    fallbackTexture = Texture::Create("TestAtlas.jpg");
    loadEntities();

}

void TestScene::loadEntities() {
    LOG_INFO("Loading entities");
    cameraEnt = registry.createEntity();
    cameraEnt.setComponent<Transform>(Transform{glm::vec3(0, 0, -2), glm::vec3(), glm::vec3(1, 1, 1)});
    cameraEnt.setComponent<CameraComponent>(CameraComponent{
            .fieldOfView = 10.0f,
            .near = 0.1f,
            .far = 100.0f,
    });

    yellowQuad = registry.createEntity();
    yellowQuad.setComponent<Transform>(Transform{glm::vec3(), glm::vec3(), glm::vec3(1, 1, 1)});
    glm::vec4 greenColor(1, 1, 0, 1);
    yellowQuad.setComponent<RenderComponent>(coloredMaterial.instantiate(&greenColor, sizeof(greenColor), {}),
                                             quadROB);

    redQuad = registry.createEntity();
    redQuad.setComponent<Transform>(Transform{glm::vec3(3, 0, 0), glm::vec3(), glm::vec3(1, 1, 1)});
    glm::vec4 redColor(0, 1, 0, 1);
    redQuad.setComponent<RenderComponent>(coloredMaterial.instantiate(&redColor, sizeof(redColor), {}),
                                          quadROB);

    texturedQuad = registry.createEntity();
    texturedQuad.setComponent<Transform>(Transform{glm::vec3(-4, 0, 0), glm::vec3(0, 0, 45), glm::vec3(1, 1, 1)});
    glm::vec4 whiteTintColor(1, 1, 1, 1);
    texturedQuad.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&whiteTintColor, sizeof(whiteTintColor), {fallbackTexture.get()}),
            quadROB);

    hexagon = registry.createEntity();
    hexagon.setComponent<Transform>(Transform{glm::vec3(0, 3, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
    glm::vec4 blueColor(0, 0, 1, 1);
    hexagon.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&whiteTintColor, sizeof(whiteTintColor), {fallbackTexture.get()}), hexROB);
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
static bool showImGuiDebugger = false;
static uint32_t selectedSceneElement = 0;

void TestScene::updateImGui() {
    ImGui::NewFrame();
    CustomImGui::ImGuiEnableDocking([&]() { imGuiMainMenu(); });

    CustomImGui::RenderLogWindow();

    ImGui::Begin("Scene");
    CustomImGui::TreeLeaf("Entity 1", 1, &selectedSceneElement);
    if (CustomImGui::TreeNodeBegin("Entity 2", 2, &selectedSceneElement)) {
        CustomImGui::TreeLeaf("Entity 2-1", 3, &selectedSceneElement);
        CustomImGui::TreeLeaf("Entity 2-2", 4, &selectedSceneElement);
        CustomImGui::TreeNodeEnd();
    }
    if (CustomImGui::TreeNodeBegin("Entity 3", 5, &selectedSceneElement)) {
        CustomImGui::TreeLeaf("Entity 3-1", 6, &selectedSceneElement);
        CustomImGui::TreeLeaf("Entity 3-2", 7, &selectedSceneElement);
        CustomImGui::TreeNodeEnd();
    }
    ImGui::End();

    const auto &fb = RenderingSystem::GetCurrentRenderer().getFramebuffer();
    auto size = CustomImGui::RenderSceneViewport(fb);

    ImGui::Begin("Info");
    ImGuiIO &io = ImGui::GetIO();
    // Basic info
    ImGui::Text("Frame: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Viewport Size: %f x %f", size.x, size.y);
    ImGui::Separator();
    if (ImGui::Button("Show ImGui Debugger")) {
        showImGuiDebugger = true;
    }
    ImGui::End();
    if (showImGuiDebugger) ImGui::ShowMetricsWindow(&showImGuiDebugger);

    if (cameraControllerActive) {
        if (ImGui::Begin("CameraControl", &cameraControllerActive)) {
            ImGui::Text("Camera Controller");
            ImGui::SliderFloat("float", &cameraSpeed, 0.0f, 50.0f);
        }
        ImGui::End();
    }
    if (itemEditActive) {
        if (ImGui::Begin("ItemEdit", &itemEditActive)) {
            ImGui::Text("Edit Entity %X", static_cast<uint32_t>(selectedSceneElement));
            auto &tc = texturedQuad.get<Transform>();
            ImGui::DragFloat3("Position", &(tc.position.x), 0.25f * dragSpeed);
            ImGui::DragFloat3("Rotation", &(tc.rotation.x), 1.0f * dragSpeed);
            ImGui::DragFloat3("Scale", &(tc.scale.x), 0.25f * dragSpeed);
            ImGui::Separator();
            ImGui::ColorEdit4("Color", &(editTintColor.r));
            if (ImGui::Button("Apply")) {
                texturedQuad.setComponent<RenderComponent>(
                        texturedMaterial.instantiate(&editTintColor, sizeof(editTintColor), {fallbackTexture.get()}),
                        quadROB);
            }
        }
        ImGui::End();
    }

}
