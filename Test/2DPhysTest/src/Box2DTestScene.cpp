#include "Box2DTestScene.h"

#include <imgui.h>

#include "JumperScript.h"

ChaosEngine::SceneConfiguration
Box2DTestScene::configure(ChaosEngine::Engine& engine) {
    LOG_INFO("Configurig Engine");
    window = &engine.getEngineWindow();
    assetManager = engine.getAssetManager();
    return ChaosEngine::SceneConfiguration{
        .rendererType = Renderer::RendererType::RENDERER2D,
        .renderSceneToOffscreenBuffer = false,
        .gravity = glm::vec2(0, -10),
    };
}

void Box2DTestScene::load() {
    using namespace Renderer;
    using namespace ChaosEngine;

    LOG_INFO("Creating materials");
    const auto BaseVertexLayout = VertexLayout{
        .binding = 0,
        .stride = sizeof(VertexPNCU),
        .inputRate = InputRate::Vertex,
        .attributes = std::vector({
            VertexAttribute{0, VertexFormat::RGB_FLOAT, offsetof(VertexPNCU, pos)},
            VertexAttribute{1, VertexFormat::RGB_FLOAT, offsetof(VertexPNCU, color)},
            VertexAttribute{2, VertexFormat::RGB_FLOAT, offsetof(VertexPNCU, normal)},
            VertexAttribute{3, VertexFormat::RG_FLOAT, offsetof(VertexPNCU, uv)},
        })
    };
    texturedMaterial = Material::Create(MaterialCreateInfo{
        .stage = ShaderPassStage::Opaque,
        .vertexLayout = BaseVertexLayout,
        .fixedFunction =
        FixedFunctionConfiguration{.depthTest = true, .depthWrite = true},
        .vertexShader = "2DSprite",
        .fragmentShader = "2DStaticTexturedSprite",
        .pushConstant = std::make_optional(Material::StandardOpaquePushConstants),
        .set0 = std::make_optional(Material::StandardOpaqueSet0),
        .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
        .set1 = std::make_optional(std::vector<ShaderBindings>(
            {
                ShaderBindings{
                    .type = ShaderBindingType::TextureSampler,
                    .stage = ShaderStage::Fragment,
                    .name = "diffuseTexture"
                },
                ShaderBindings{
                    .type = ShaderBindingType::UniformBuffer,
                    .stage = ShaderStage::Fragment,
                    .name = "materialData",
                    .layout = std::make_optional(std::vector<ShaderBindingLayout>({
                        ShaderBindingLayout{
                            .type = ShaderValueType::Vec4,
                            .name = "color"
                        },
                    }))
                }
            })),
        .set1ExpectedCount = 64,
        .name = "TexturedSprite",
    });
    assetManager->registerMaterial(
        "TexturedSprite", texturedMaterial,
        AssetManager::MaterialInfo{.hasTintColor = true});

    LOG_INFO("Creating quad buffers");
    auto quadAsset = ModelLoader::getQuad_PNCU();
    auto quadVB = Buffer::Create(quadAsset.vertices.data(),
                                 quadAsset.vertices.size() * sizeof(VertexPNCU),
                                 BufferType::Vertex);
    auto quadIB = Buffer::Create(quadAsset.indices.data(),
                                 quadAsset.indices.size() * sizeof(uint32_t),
                                 BufferType::Index);
    quadROB = RenderMesh::Create(std::move(quadVB), std::move(quadIB),
                                 quadAsset.indices.size());
    assetManager->registerMesh("Quad", quadROB, AssetManager::MeshInfo{});

    LOG_INFO("Loading base textures");

    auto fallbackTex = Texture::Create("TestAtlas.jpg");
    assetManager->registerTexture("TestAtlas.jpg", std::move(fallbackTex),
                                  AssetManager::TextureInfo{});
    auto plainTex = Texture::Create("noTex.jpg");
    assetManager->registerTexture("Square", std::move(plainTex),
                                  AssetManager::TextureInfo{});

    LOG_INFO("Loading base font");
    assetManager->loadFont("OpenSauceSans", "fonts/OpenSauceSans-Regular.ttf",
                           FontStyle::Regular, 16.0f, 95.0f);
    assetManager->loadFont("OpenSauceSans", "fonts/OpenSauceSans-Italic.ttf",
                           FontStyle::Italic, 16.0f, 95.0f);
    assetManager->loadFont("OpenSauceSans", "fonts/OpenSauceSans-Bold.ttf",
                           FontStyle::Bold, 16.0f, 95.0f);
    loadEntities();
}

void Box2DTestScene::loadEntities() {
    using namespace ChaosEngine;
    LOG_INFO("Loading entities");
    mainCamera = createEntity();
    mainCamera.setComponent<Transform>(Transform{glm::vec3(0, 0, -2), glm::vec3(), glm::vec3(1, 1, 1)});
    mainCamera.setComponent<CameraComponent>(CameraComponent{
        .fieldOfView = 10.0f,
        .near = 0.1f,
        .far = 100.0f,
        .active = true,
        .mainCamera = true,
    });

    auto floor = createEntity();
    floor.setComponent<Meta>(Meta{"Floor"});
    floor.setComponent<Transform>(
        Transform{glm::vec3(0, -8.0f, 0), glm::vec3(), glm::vec3(5, 1, 1)});
    glm::vec4 redColor(1, 0, 0, 1);
    floor.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&redColor, sizeof(redColor), {&assetManager->getTexture("TestAtlas.jpg")}),
        quadROB);
    floor.setComponent<StaticRigidBodyComponent>(RigidBody2D::CreateStaticRigidBody(floor, floor.get<Transform>()));

    auto ball = createEntity();
    ball.setComponent<Meta>(Meta{"Gravity test"});
    ball.setComponent<Transform>(
        Transform{glm::vec3(0, 6.0f, 0), glm::vec3(0, 0, 40), glm::vec3(1, 1, 1)});
    glm::vec4 whiteColor(1, 1, 1, 1);
    ball.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&whiteColor, sizeof(whiteColor), {&assetManager->getTexture("Square")}),
        quadROB);
    ball.setComponent<DynamicRigidBodyComponent>(
        RigidBody2D::CreateDynamicRigidBody(ball, ball.get<Transform>(), 1, 0.3f, true));


    auto floor1 = createEntity();
    floor1.setComponent<Meta>(Meta{"Floor 2"});
    floor1.setComponent<Transform>(
        Transform{glm::vec3(-8, -7.0f, 0), glm::vec3(), glm::vec3(2, 1, 1)});
    floor1.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&whiteColor, sizeof(whiteColor), {&assetManager->getTexture("TestAtlas.jpg")}),
        quadROB);
    floor1.setComponent<StaticRigidBodyComponent>(RigidBody2D::CreateStaticRigidBody(floor1, floor1.get<Transform>()));
    auto jumper = createEntity();
    jumper.setComponent<Meta>(Meta{"Gravity test"});
    jumper.setComponent<Transform>(
        Transform{glm::vec3(-8, 7.0f, 0), glm::vec3(), glm::vec3(1)});
    jumper.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&whiteColor, sizeof(whiteColor), {&assetManager->getTexture("Square")}),
        quadROB);
    jumper.setComponent<DynamicRigidBodyComponent>(
        RigidBody2D::CreateDynamicRigidBody(jumper, jumper.get<Transform>(), 1, 0.3f, true));
    auto jumperScript = std::unique_ptr<NativeScript>(new JumperScript(jumper, 14.0f));
    jumper.setComponent<NativeScriptComponent>(std::move(jumperScript), true);
    LOG_INFO("Jumper entity has ID {}", static_cast<uint32_t>(jumper));
}

void Box2DTestScene::update(float /*deltaTime*/) {
    // Close window controls
    if (window->isKeyDown(GLFW_KEY_ESCAPE) ||
        (window->isKeyDown(GLFW_KEY_Q) && window->isKeyDown(GLFW_KEY_LEFT_CONTROL))) { window->close(); }
}

void Box2DTestScene::updateImGui() {
    ImGui::NewFrame();

    ImGui::Begin("Info");
    ImGuiIO& io = ImGui::GetIO();
    // Basic info
    ImGui::Text("Frame: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Viewport Size: %f x %f", io.DisplaySize.x, io.DisplaySize.y);
    ImGui::End();
    // const auto &fb = ChaosEngine::RenderingSystem::GetCurrentRenderer().getFramebuffer();
    // auto size = CustomImGui::CoreImGui::RenderSceneViewport(fb, "Scene", &viewportInFocus);
}
