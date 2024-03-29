#include "TestScene.h"

#include "Sandbox/src/common/CustomImGui.h"
#include "Sandbox/src/common/AssetView.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace ChaosEngine;

ChaosEngine::SceneConfiguration TestScene::configure(Engine &engine) {
    window = &engine.getEngineWindow();
    return ChaosEngine::SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D,
            .renderSceneToOffscreenBuffer = true,
    };
}


void TestScene::load() {
    using namespace Renderer;
    // Load meshes
    LOG_INFO("Creating quad buffers");
    auto quadAsset = ModelLoader::getQuad_PNCU();
    auto vertexBuffer = Buffer::Create(quadAsset.vertices.data(), quadAsset.vertices.size() * sizeof(VertexPNCU),
                                       BufferType::Vertex);
    auto indexBuffer = Buffer::Create(quadAsset.indices.data(), quadAsset.indices.size() * sizeof(uint32_t),
                                      BufferType::Index);
    quadROB = RenderMesh::Create(std::move(vertexBuffer), std::move(indexBuffer), quadAsset.indices.size());

    LOG_INFO("Creating hex buffers");
    auto hexAsset = ModelLoader::getHexagon();
    auto hexVertexBuffer = Buffer::Create(hexAsset.vertices.data(), hexAsset.vertices.size() * sizeof(VertexPNCU),
                                          BufferType::Vertex);
    auto hexIndexBuffer = Buffer::Create(hexAsset.indices.data(), hexAsset.indices.size() * sizeof(uint32_t),
                                         BufferType::Index);
    hexROB = RenderMesh::Create(std::move(hexVertexBuffer), std::move(hexIndexBuffer), hexAsset.indices.size());

    LOG_INFO("Creating materials");
    const auto BaseVertexLayout = VertexLayout{.binding = 0, .stride = sizeof(VertexPNCU), .inputRate=InputRate::Vertex,
            .attributes = std::vector<VertexAttribute>(
                    {
                            VertexAttribute{0, VertexFormat::RGB_FLOAT, offsetof(VertexPNCU, pos)},
                            VertexAttribute{1, VertexFormat::RGB_FLOAT, offsetof(VertexPNCU, color)},
                            VertexAttribute{2, VertexFormat::RGB_FLOAT, offsetof(VertexPNCU, normal)},
                            VertexAttribute{3, VertexFormat::RG_FLOAT, offsetof(VertexPNCU, uv)},
                    })};
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
            .vertexLayout = BaseVertexLayout,
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
            .vertexLayout = BaseVertexLayout,
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
    editorCamera = createEntity();
    editorCamera.setComponent<Transform>(Transform{glm::vec3(0, 0, -2), glm::vec3(), glm::vec3(1, 1, 1)});
    editorCamera.setComponent<CameraComponent>(CameraComponent{
            .fieldOfView = 10.0f,
            .near = 0.1f,
            .far = 100.0f,
            .active = true,
            .mainCamera = true,
    });

    auto yellowQuad = createEntity();
    yellowQuad.setComponent<Meta>(Meta{"Yellow quad"});
    yellowQuad.setComponent<Transform>(Transform{glm::vec3(), glm::vec3(), glm::vec3(1, 1, 1)});
    glm::vec4 greenColor(1, 1, 0, 1);
    yellowQuad.setComponent<RenderComponent>(coloredMaterial.instantiate(&greenColor, sizeof(greenColor), {}),
                                             quadROB);

    auto greenQuad = createEntity();
    greenQuad.setComponent<Meta>(Meta{"Green quad"});
    greenQuad.setComponent<Transform>(Transform{glm::vec3(3, 0, 0), glm::vec3(), glm::vec3(1, 1, 1)});
    glm::vec4 redColor(0, 1, 0, 1);
    greenQuad.setComponent<RenderComponent>(coloredMaterial.instantiate(&redColor, sizeof(redColor), {}),
                                            quadROB);

    auto texturedQuad = createEntity();
    texturedQuad.setComponent<Meta>(Meta{"Textured quad"});
    texturedQuad.setComponent<Transform>(Transform{glm::vec3(-4, 0, 0), glm::vec3(0, 0, 45), glm::vec3(1, 1, 1)});
    glm::vec4 whiteTintColor(1, 1, 1, 1);
    texturedQuad.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&whiteTintColor, sizeof(whiteTintColor), {fallbackTexture.get()}),
            quadROB);

    auto hexagon = createEntity();
    hexagon.setComponent<Meta>(Meta{"Textured hexagon"});
    hexagon.setComponent<Transform>(Transform{glm::vec3(0, 3, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
    glm::vec4 blueColor(0, 0, 1, 1);
    hexagon.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&whiteTintColor, sizeof(whiteTintColor), {fallbackTexture.get()}), hexROB);
}

// Test data
static bool cameraControllerActive = false;
static bool viewportInFocus = false;
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
    if (viewportInFocus) {
        if (window->isKeyDown(GLFW_KEY_W)) { origin.y -= cameraSpeed * deltaTime; }
        if (window->isKeyDown(GLFW_KEY_S)) { origin.y += cameraSpeed * deltaTime; }
        if (window->isKeyDown(GLFW_KEY_A)) { origin.x += cameraSpeed * deltaTime; }
        if (window->isKeyDown(GLFW_KEY_D)) { origin.x -= cameraSpeed * deltaTime; }
        if (window->isKeyDown(GLFW_KEY_KP_ADD)) {
            editorCamera.get<CameraComponent>().fieldOfView -= 5 * deltaTime;
        }
        if (window->isKeyDown(GLFW_KEY_KP_SUBTRACT)) {
            editorCamera.get<CameraComponent>().fieldOfView += 5 * deltaTime;
        }

        editorCamera.get<Transform>().position = origin;
    }

}

void TestScene::addNewEntity() {
    auto entity = ecs.addEntity();
    entity.setComponent<Meta>(Meta{"New Entity"});
    entity.setComponent<Transform>(Transform{glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
    glm::vec4 whiteTintColor(1, 1, 1, 1);
    entity.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&whiteTintColor, sizeof(whiteTintColor), {fallbackTexture.get()}),
            quadROB);
}

void TestScene::imGuiMainMenu() {
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

static glm::vec4 editTintColor = glm::vec4(1, 1, 1, 1);
static bool showImGuiDebugger = false;
static bool showImGuiDemo = false;
static ECS::entity_t selectedSceneElement = ECS::null;
// Sanity check because the editor uses uint32_t to manage the entity selection
static_assert(std::is_same<uint32_t, std::underlying_type<entt::entity>::type>::value,
              "EnTT entity type does not match editor entity type!");

void TestScene::updateImGui() {
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
    ImGui::Text("Viewport Size: %f x %f", size.second.x - size.first.x, size.second.y - size.first.y);
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

                ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
                auto &meta = entity.get<Meta>();
                ImGui::InputText(" ", &meta.name, input_text_flags);

                const auto panelWidth = ImGui::GetWindowWidth();
                ImGui::SameLine(panelWidth - 60);
                if (ImGui::Button("Delete")) {
                    ecs.removeEntity(entity);
                    selectedSceneElement = ECS::null;
                } else {

                    ImGui::Separator();

                    auto &tc = entity.get<Transform>();
                    ImGui::DragFloat3("Position", &(tc.position.x), 0.25f * dragSpeed);
                    ImGui::DragFloat3("Rotation", &(tc.rotation.x), 1.0f * dragSpeed);
                    ImGui::DragFloat3("Scale", &(tc.scale.x), 0.25f * dragSpeed);
                    ImGui::Separator();
                    ImGui::ColorEdit4("Color", &(editTintColor.r));
                    if (ImGui::Button("Apply")) {
                        entity.setComponent<RenderComponent>(
                                texturedMaterial.instantiate(&editTintColor, sizeof(editTintColor),
                                                             {fallbackTexture.get()}),
                                quadROB);
                    }
                }
            } else {
                ImGui::Text("No Item selected");
            }
        }
        ImGui::End();
    }

    AssetView::renderAssetView();
}
