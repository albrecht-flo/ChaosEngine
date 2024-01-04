#include "SoundTestScene.h"

#include <imgui.h>

#include "CameraScript.h"
#include "SoundButtonScript.h"

ChaosEngine::SceneConfiguration
SoundTestScene::configure(ChaosEngine::Engine &engine) {
    LOG_INFO("Configuring Engine");
    this->engine = &engine;
    window = &engine.getEngineWindow();
    assetManager = engine.getAssetManager();
    return ChaosEngine::SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D,
            .renderSceneToOffscreenBuffer = false,
            .debugRenderingEnabled = true,
    };
}

void SoundTestScene::load() {
    using namespace Renderer;
    using namespace ChaosEngine;

    LOG_INFO("Creating materials"); // ---------------------------------------------------------------------------------
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

    auto uiMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .vertexLayout = VertexLayout{.binding = 0, .stride = sizeof(VertexPCU), .inputRate=InputRate::Vertex,
                    .attributes = std::vector<VertexAttribute>(
                            {
                                    VertexAttribute{0, VertexFormat::RGB_FLOAT, offsetof(VertexPCU, pos)},
                                    VertexAttribute{1, VertexFormat::RGBA_FLOAT, offsetof(VertexPCU, color)},
                                    VertexAttribute{2, VertexFormat::RG_FLOAT, offsetof(VertexPCU, uv)},
                            })},
            .fixedFunction = FixedFunctionConfiguration{.depthTest = true, .depthWrite = true, .alphaBlending=true},
            .vertexShader = "ENGINE_UIBase",
            .fragmentShader = "UI",
            .pushConstant = std::make_optional(Material::StandardOpaquePushConstants),
            .set0 = std::make_optional(Material::StandardOpaqueSet0),
            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
            .set1 = std::make_optional(std::vector<ShaderBindings>(
                    {
                            ShaderBindings{.type = ShaderBindingType::TextureSampler, .stage=ShaderStage::Fragment, .name="texture"},
                            ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Fragment, .name="materialData",
                                    .layout=std::make_optional(std::vector<ShaderBindingLayout>(
                                            {
                                                    ShaderBindingLayout{.type = ShaderValueType::Vec4, .name ="color"},
                                            }))
                            }
                    })),
            .set1ExpectedCount = 64,
            .name="UIMaterial",
    });
    assetManager->registerMaterial("UIMaterial", uiMaterial, AssetManager::MaterialInfo{.hasTintColor=true});

    LOG_INFO("Creating quad buffers"); // ------------------------------------------------------------------------------
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

    LOG_INFO("Creating UI quad buffers"); // ---------------------------------------------------------------------------
    auto uiQuadAsset = ModelLoader::getQuad_PCU();
    auto uiQuadVertexBuffer = Buffer::Create(uiQuadAsset.vertices.data(),
                                             uiQuadAsset.vertices.size() * sizeof(VertexPCU), BufferType::Vertex);
    auto uiQuadIndexBuffer = Buffer::Create(uiQuadAsset.indices.data(), uiQuadAsset.indices.size() * sizeof(uint32_t),
                                            BufferType::Index);
    auto uiQuadROB = RenderMesh::Create(std::move(uiQuadVertexBuffer), std::move(uiQuadIndexBuffer),
                                        uiQuadAsset.indices.size());
    assetManager->registerMesh("UI/Quad", std::move(uiQuadROB), AssetManager::MeshInfo{});

    LOG_INFO("Loading base textures"); // ------------------------------------------------------------------------------

    auto plainTex = Texture::Create("noTex.jpg");
    assetManager->registerTexture("Square", std::move(plainTex),
                                  AssetManager::TextureInfo{});

    LOG_INFO("Loading base font"); // ----------------------------------------------------------------------------------
    assetManager->loadFont("OpenSauceSans", "fonts/OpenSauceSans-Regular.ttf",
                           FontStyle::Regular, 16.0f, 95.0f);
    assetManager->loadFont("OpenSauceSans", "fonts/OpenSauceSans-Italic.ttf",
                           FontStyle::Italic, 16.0f, 95.0f);
    assetManager->loadFont("OpenSauceSans", "fonts/OpenSauceSans-Bold.ttf",
                           FontStyle::Bold, 16.0f, 95.0f);

    backgroundAudioBuffer = AudioBuffer::Create("sounds/background.ogg");
    stepsAudioBuffer = AudioBuffer::Create("sounds/steps.ogg");

    loadEntities();
}

static bool surroundRunning = false;
static glm::vec3 surroundOriginalPosition{};
static float path = 0.0f;
static float surroundSpeed = 0.7f;

void SoundTestScene::loadEntities() {
    using namespace ChaosEngine;

    LOG_INFO("Loading entities");
    mainCamera = createEntity();
    auto mainCameraTransform = Transform{glm::vec3(0, 0, -2), glm::vec3(), glm::vec3(1, 1, 1)};
    mainCamera.setComponent<Transform>(mainCameraTransform);
    mainCamera.setComponent<CameraComponent>(CameraComponent{
            .fieldOfView = 10.0f,
            .near = 0.1f,
            .far = 100.0f,
            .active = true,
            .mainCamera = true,
    });
    auto script = std::unique_ptr<ChaosEngine::NativeScript>(new CameraScript(mainCamera));
    mainCamera.setComponent<NativeScriptComponent>(std::move(script), true);
    mainCamera.setComponent<AudioListenerComponent>(true, mainCameraTransform.position);
    auto backgroundAudioSource = AudioSource::Create(mainCameraTransform.position, false);
    backgroundAudioSource.setGain(0.66f);
    mainCamera.setComponent<AudioSourceComponent>(
            std::move(backgroundAudioSource),
            mainCameraTransform.position);
    mainCamera.get<AudioSourceComponent>().source.setBuffer(backgroundAudioBuffer);

    const glm::vec4 whiteColor(1, 1, 1, 1);
    const glm::vec4 redColor(1, 0, 0, 1);

    auto background = createEntity();
    background.setComponent<Meta>(Meta{"Background"});
    background.setComponent<Transform>(
            Transform{glm::vec3(0, 0, -10), glm::vec3(), glm::vec3(20, 16, 0.1)});
    const glm::vec4 greyColor(0.66f, 0.66f, 0.70f, 1);
    background.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&greyColor, sizeof(greyColor), {&assetManager->getTexture("Square")}),
            quadROB);


    audioTesterSurround = createEntity();
    auto audioTesterTransform = Transform{glm::vec3(0, 0, -2), glm::vec3(), glm::vec3(1, 1, 1)};
    surroundOriginalPosition = audioTesterTransform.position;
    audioTesterSurround.setComponent<Meta>(Meta{"Steps"});
    audioTesterSurround.setComponent<Transform>(audioTesterTransform);
    audioTesterSurround.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&redColor, sizeof(redColor), {&assetManager->getTexture("Square")}),
            quadROB);
    auto audioTSource = AudioSource::Create(audioTesterTransform.position, true);
    audioTSource.setBuffer(stepsAudioBuffer);
    audioTSource.setGain(7.0f);
    audioTesterSurround.setComponent<AudioSourceComponent>(std::move(audioTSource), audioTesterTransform.position);

    glm::vec3 positions[] = {glm::vec3(0, 0, 0), glm::vec3(-2, 0, -2), glm::vec3(2, 0, -2), glm::vec3(0, 0, -4)};
    for (int i = 0; i < spatialTesters.max_size(); ++i) {
        const glm::vec4 spatialColor(0, 0.1f, 0.66f, 1);
        auto transform = Transform{positions[i], glm::vec3(), glm::vec3(1, 1, 1)};

        auto entity = createEntity();
        entity.setComponent<Meta>(Meta{"Spatial test" + std::to_string(i)});
        entity.setComponent<Transform>(transform);

        auto audioSource = AudioSource::Create(transform.position, false);
        audioSource.setBuffer(stepsAudioBuffer);
        audioSource.setGain(2.0f);
        entity.setComponent<AudioSourceComponent>(std::move(audioSource), transform.position);

        spatialTesters[i] = std::move(entity);
    }

    const glm::vec4 buttonColor{0.5f, 0.5f, 1.0f, 1};
    const std::string uiMeshName = "UI/Quad";
    auto uiMesh = assetManager->getMesh(uiMeshName);
    auto uiMaterial = assetManager->getMaterial("UIMaterial");

    auto button = createEntity();
    button.setComponent<Meta>("Test Button");
    button.setComponent<Transform>(
            Transform{glm::vec3{800, 128, 1}, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
    button.setComponent<UIRenderComponent>(UIRenderComponent{
            .materialInstance = uiMaterial.instantiate(&buttonColor, sizeof(buttonColor),
                                                       {&assetManager->getTexture("Square")}),
            .mesh = uiMesh,
            .scaleOffset = glm::vec3(0, 0, 0),
    });
    button.setComponent<UIComponent>(
            UIComponent{.active = true, .offsetPosition={35, 15, -0.1f}, .offsetRotation{0},
                    .offsetScale{50, 20, 1}});
    button.setComponent<UITextComponent>(UITextComponent{
            .font = assetManager->loadFont("OpenSauceSans", "fonts/OpenSauceSans-Bold.ttf", FontStyle::Bold,
                                           16.0f, 72.0f),
            .style = FontStyle::Bold,
            .textColor = glm::vec4(0.33f, 0.33f, 0.33f, 1),
            .text = "Click me",
    });

    auto buttonScript = std::unique_ptr<ChaosEngine::NativeScript>(new SoundButtonScript(button, mainCamera));
    button.setComponent<NativeScriptComponent>(std::move(buttonScript), true);

}

void SoundTestScene::update(float deltaTime) {
    // Close window controls
    if (window->isKeyDown(GLFW_KEY_ESCAPE) ||
        (window->isKeyDown(GLFW_KEY_Q) && window->isKeyDown(GLFW_KEY_LEFT_CONTROL))) { window->close(); }

    if (surroundRunning) {
        const auto &center = mainCamera.get<Transform>().position;
        path += deltaTime * surroundSpeed;
        glm::vec3 newPos = glm::rotate(glm::qua(glm::vec3{0, path, 0}), center + surroundOriginalPosition) - center;
        audioTesterSurround.get<Transform>().position = newPos;
        LOG_DEBUG("New position ({}, {}, {})", newPos.x, newPos.y, newPos.z);
    }
}

static bool looping = false;

void SoundTestScene::updateImGui() {
    using namespace ChaosEngine;

    ImGui::NewFrame();

    ImGui::Begin("Info");
    ImGuiIO &io = ImGui::GetIO();
    // Basic info
    ImGui::Text("Frame: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Viewport Size: %f x %f", io.DisplaySize.x, io.DisplaySize.y);
    ImGui::Separator();

    ImGui::Text("Audio Objects");
    Transform listenerInfo = AudioSystem::GetListenerPosition();
    ImGui::Text("Listener Position (%.2f, %.2f, %.2f)", listenerInfo.position.x, listenerInfo.position.y,
                listenerInfo.position.z);
    const auto &surroundTesterPos = audioTesterSurround.get<Transform>().position;
    ImGui::Text("Surround position (%.2f, %.2f, %.2f)", surroundTesterPos.x, surroundTesterPos.y, surroundTesterPos.z);
    for (int i = 0; i < spatialTesters.size(); ++i) {
        const auto &pos = spatialTesters[i].get<Transform>().position;
        ImGui::Text("Spatial Tester %d position (%.2f, %.2f, %.2f)", i, pos.x, pos.y, pos.z);
    }

    ImGui::Separator();
    auto &bgAudioSource = mainCamera.get<AudioSourceComponent>().source;
    ImGui::Text("Audio controls");
    ImGui::Columns(3, "background_ctrls");

    if (ImGui::Button("Play bg"))
        bgAudioSource.play();
    ImGui::NextColumn();
    if (ImGui::Button("Pause bg"))
        bgAudioSource.pause();
    ImGui::NextColumn();
    if (ImGui::Button("Stop bg"))
        bgAudioSource.stop();

    ImGui::Columns(2);
    ImGui::Separator();
    ImGui::Text("BG looping:");
    ImGui::NextColumn();
    ImGui::Text("%s", (looping ? "ON" : "OFF"));
    ImGui::NextColumn();
    if (ImGui::Button("Toggle bg looping")) {
        looping = !looping;
        bgAudioSource.setLooping(looping);
    }
    ImGui::NextColumn();
    ImGui::NextColumn();

    ImGui::Separator();
    const auto &bgBuffer = bgAudioSource.getBuffer();
    auto sourcePos = bgAudioSource.getBufferPosition() / bgBuffer.getSampleSize();
    auto bufferLength = bgBuffer.getSamples() * bgBuffer.getSampleSize();
    float totalSeconds = (float) ((bgBuffer.getSamples() / bgBuffer.getSampleRate()));
    ImGui::Text("Time:");
    ImGui::NextColumn();
    ImGui::Text("%.1fs /%.1fs",
                totalSeconds * (((float) sourcePos / (float) bgBuffer.getSamples()) / bgBuffer.getChannels()),
                totalSeconds);
    ImGui::NextColumn();
    ImGui::Text("BG buffer offset:");
    ImGui::NextColumn();
    ImGui::Text("%lld", sourcePos);
    ImGui::NextColumn();
    ImGui::Text("BG buffer length:");
    ImGui::NextColumn();
    ImGui::Text("%lld", bufferLength);
    ImGui::NextColumn();

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::Text("Start/Stop surround tester");
    if (ImGui::Button("Start/Stop")) {
        surroundRunning = !surroundRunning;
        auto &source = audioTesterSurround.get<AudioSourceComponent>().source;
        if (surroundRunning) {
            source.play();
        } else {
            source.stop();
        }
    }
    ImGui::SliderFloat("Rotation speed", &surroundSpeed, 0, 5, "%.2f");

    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImGui::Columns(3);
    ImGui::NextColumn();
    if (ImGui::Button("Play Front"))
        spatialTesters[0].get<AudioSourceComponent>().source.play();
    ImGui::NextColumn();
    ImGui::NextColumn();

    if (ImGui::Button("Play Left"))
        spatialTesters[1].get<AudioSourceComponent>().source.play();
    ImGui::NextColumn();
    ImGui::NextColumn();
    if (ImGui::Button("Play Right"))
        spatialTesters[2].get<AudioSourceComponent>().source.play();
    ImGui::NextColumn();

    ImGui::NextColumn();
    if (ImGui::Button("Play Behind"))
        spatialTesters[3].get<AudioSourceComponent>().source.play();
    ImGui::NextColumn();

    ImGui::NextColumn();
    ImGui::Columns(1);

    ImGui::End();
}
