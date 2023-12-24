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
    auto ballTex = Texture::Create("ball.png");
    assetManager->registerTexture("ball", std::move(ballTex),
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
    using RigitBody2DShapeType = RigidBody2D::RigitBody2DShapeType;
    using RigitBody2DShape = RigidBody2D::RigitBody2DShape;

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

    auto background = createEntity();
    background.setComponent<Meta>(Meta{"Background"});
    background.setComponent<Transform>(
        Transform{glm::vec3(0, 0, 0), glm::vec3(), glm::vec3(20, 16, 0.1)});
    const glm::vec4 greyColor(0.66f, 0.66f, 0.70f, 1);
    background.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&greyColor, sizeof(greyColor), {&assetManager->getTexture("Square")}),
        quadROB);

    auto floor = createEntity();
    floor.setComponent<Meta>(Meta{"Floor"});
    floor.setComponent<Transform>(
        Transform{glm::vec3(0, -8.0f, 0), glm::vec3(), glm::vec3(5, 1, 1)});
    const glm::vec4 redColor(1, 0, 0, 1);
    floor.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&redColor, sizeof(redColor), {&assetManager->getTexture("TestAtlas.jpg")}),
        quadROB);
    floor.setComponent<StaticRigidBodyComponent>(RigidBody2D::CreateStaticRigidBody(floor,
        RigitBody2DShape{RigitBody2DShapeType::Box, glm::vec2{5, 1}}));

    const glm::vec4 whiteColor(1, 1, 1, 1);

    auto square = createEntity();
    square.setComponent<Meta>(Meta{"Gravity test"});
    square.setComponent<Transform>(
        Transform{glm::vec3(0, 6.0f, 0), glm::vec3(0, 0, 40), glm::vec3(1, 1, 1)});
    square.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&whiteColor, sizeof(whiteColor), {&assetManager->getTexture("Square")}),
        quadROB);
    square.setComponent<DynamicRigidBodyComponent>(
        RigidBody2D::CreateDynamicRigidBody(square, RigitBody2DShape{RigitBody2DShapeType::Box, glm::vec2{1, 1}}, 1,
                                            0.3f, true));


    auto floor1 = createEntity();
    floor1.setComponent<Meta>(Meta{"Floor 2"});
    floor1.setComponent<Transform>(
        Transform{glm::vec3(-8, -7.0f, 0), glm::vec3(), glm::vec3(2, 1, 1)});
    floor1.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&whiteColor, sizeof(whiteColor), {&assetManager->getTexture("TestAtlas.jpg")}),
        quadROB);
    floor1.setComponent<StaticRigidBodyComponent>(
        RigidBody2D::CreateStaticRigidBody(floor1, RigitBody2DShape{RigitBody2DShapeType::Box, glm::vec2{2, 1}}));

    auto jumper = createEntity();
    jumper.setComponent<Meta>(Meta{"Gravity test"});
    jumper.setComponent<Transform>(
        Transform{glm::vec3(-8, 7.0f, 0), glm::vec3(), glm::vec3(1)});
    jumper.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&whiteColor, sizeof(whiteColor), {&assetManager->getTexture("Square")}),
        quadROB);
    jumper.setComponent<DynamicRigidBodyComponent>(
        RigidBody2D::CreateDynamicRigidBody(jumper, RigitBody2DShape{RigitBody2DShapeType::Box, glm::vec2{1}}, 1, 0.3f,
                                            true));
    auto jumperScript = std::unique_ptr<NativeScript>(new JumperScript(jumper, 14.0f));
    jumper.setComponent<NativeScriptComponent>(std::move(jumperScript), true);


    // -----------------------------------------------------------------------------------------------------------------
    const glm::vec4 blueColor(0, 0, 0.7f, 1);
    auto funnelL = createEntity();
    funnelL.setComponent<Meta>(Meta{"Funnel left"});
    funnelL.setComponent<Transform>(
        Transform{glm::vec3(9.1f, -5.0f, 0), glm::vec3(0, 0, -45), glm::vec3(4, 1, 1)});
    funnelL.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&blueColor, sizeof(blueColor), {&assetManager->getTexture("Square")}),
        quadROB);
    funnelL.setComponent<StaticRigidBodyComponent>(
        RigidBody2D::CreateStaticRigidBody(funnelL, RigitBody2DShape{RigitBody2DShapeType::Box, glm::vec2{4, 1}}));
    auto funnelR = createEntity();
    funnelR.setComponent<Meta>(Meta{"Funnel right"});
    funnelR.setComponent<Transform>(
        Transform{glm::vec3(13.1f, -5.0f, 0), glm::vec3(0, 0, 45), glm::vec3(4, 1, 1)});
    funnelR.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&blueColor, sizeof(blueColor), {&assetManager->getTexture("Square")}),
        quadROB);
    funnelR.setComponent<StaticRigidBodyComponent>(
        RigidBody2D::CreateStaticRigidBody(funnelR, RigitBody2DShape{RigitBody2DShapeType::Box, glm::vec2{4, 1}}));

    auto ball = createEntity();
    ball.setComponent<Meta>(Meta{"Ball"});
    ball.setComponent<Transform>(
        Transform{glm::vec3(14, 7.0f, 0), glm::vec3(), glm::vec3(0.5f, 0.5f, 1)});
    ball.setComponent<RenderComponent>(
        texturedMaterial.instantiate(&whiteColor, sizeof(whiteColor), {&assetManager->getTexture("ball")}),
        quadROB);
    ball.setComponent<DynamicRigidBodyComponent>(
        RigidBody2D::CreateDynamicRigidBody(ball, RigitBody2DShape{RigitBody2DShapeType::Cricle, glm::vec2{0.5}}, 3,
                                            1.8f, true));
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
