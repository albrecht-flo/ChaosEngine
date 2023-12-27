#include "Box2DTestScene.h"

#include <imgui.h>

#include "CameraScript.h"
#include "JumperScript.h"
#include "PusherScript.h"

ChaosEngine::SceneConfiguration
Box2DTestScene::configure(ChaosEngine::Engine &engine) {
    LOG_INFO("Configuring Engine");
    this->engine = &engine;
    window = &engine.getEngineWindow();
    assetManager = engine.getAssetManager();
    return ChaosEngine::SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D,
            .renderSceneToOffscreenBuffer = false,
            .gravity = glm::vec2(0, -10),
            .debugRenderingEnabled = true,
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
                                    .layout = std::make_optional(
                                            std::vector<ShaderBindingLayout>({
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
    auto script = std::unique_ptr<ChaosEngine::NativeScript>(new CameraScript(mainCamera));
    mainCamera.setComponent<NativeScriptComponent>(std::move(script), true);

    const glm::vec4 whiteColor(1, 1, 1, 1);

    auto background = createEntity();
    background.setComponent<Meta>(Meta{"Background"});
    background.setComponent<Transform>(
            Transform{glm::vec3(0, 0, 0), glm::vec3(), glm::vec3(20, 16, 0.1)});
    const glm::vec4 greyColor(0.66f, 0.66f, 0.70f, 1);
    background.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&greyColor, sizeof(greyColor), {&assetManager->getTexture("Square")}),
            quadROB);

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
            RigidBody2D::CreateDynamicRigidBody(jumper, RigitBody2DShape{RigitBody2DShapeType::Box, glm::vec2{1}}, 1,
                                                0.3f,
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

    // -----------------------------------------------------------------------------------------------------------------
    glm::vec4 darkGrey{0.33, 0.5, 0.33, 1};
    auto slope1 = createEntity();
    slope1.setComponent<Meta>(Meta{"Slope 1"});
    slope1.setComponent<Transform>(
            Transform{glm::vec3(-1.5, 3.0f, 0), glm::vec3(0, 0, 30), glm::vec3(0.5f, 2, 1)});
    slope1.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&darkGrey, sizeof(darkGrey), {&assetManager->getTexture("Square")}),
            quadROB);
    slope1.setComponent<DynamicRigidBodyComponent>(RigidBody2D::CreateKineticRigidBody(slope1,
                                                                                       RigitBody2DShape{
                                                                                               RigitBody2DShapeType::Box,
                                                                                               glm::vec2{0.5, 2}}));
    auto rotationScript1 = std::unique_ptr<NativeScript>(new PusherScript(slope1));
    slope1.setComponent<NativeScriptComponent>(std::move(rotationScript1), true);

    auto slope2 = createEntity();
    slope2.setComponent<Meta>(Meta{"Slope 2"});
    slope2.setComponent<Transform>(
            Transform{glm::vec3(3, 3.0f, 0), glm::vec3(0, 0, 30), glm::vec3(5, 0.5f, 1)});
    slope2.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&darkGrey, sizeof(darkGrey), {&assetManager->getTexture("Square")}),
            quadROB);
    slope2.setComponent<StaticRigidBodyComponent>(RigidBody2D::CreateStaticRigidBody(slope2,
                                                                                     RigitBody2DShape{
                                                                                             RigitBody2DShapeType::Box,
                                                                                             glm::vec2{5, 0.5}}));

    auto ball2 = createEntity();
    ball2.setComponent<Meta>(Meta{"Push tester"});
    ball2.setComponent<Transform>(
            Transform{glm::vec3(1, 5.0f, 0), glm::vec3(0, 0, 0), glm::vec3(0.5, 0.5, 0.5)});
    ball2.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&whiteColor, sizeof(whiteColor), {&assetManager->getTexture("ball")}),
            quadROB);
    ball2.setComponent<DynamicRigidBodyComponent>(
            RigidBody2D::CreateDynamicRigidBody(ball2, RigitBody2DShape{RigitBody2DShapeType::Cricle, glm::vec2{0.5}},
                                                2,
                                                1.33f, true));
}

void Box2DTestScene::update(float /*deltaTime*/) {
    // Close window controls
    if (window->isKeyDown(GLFW_KEY_ESCAPE) ||
        (window->isKeyDown(GLFW_KEY_Q) && window->isKeyDown(GLFW_KEY_LEFT_CONTROL))) { window->close(); }
}

void Box2DTestScene::updateImGui() {
    ImGui::NewFrame();

    ImGui::Begin("Info");
    ImGuiIO &io = ImGui::GetIO();
    // Basic info
    ImGui::Text("Frame: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Viewport Size: %f x %f", io.DisplaySize.x, io.DisplaySize.y);
    ImGui::Separator();
    if (ImGui::Button("Toggle BoundingBoxes")) {
        engine->setPhysicsDebug(!engine->getPhysicsDebug());
    }
    ImGui::End();
}
