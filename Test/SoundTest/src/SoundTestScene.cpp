#include "SoundTestScene.h"

#include <imgui.h>

#include "CameraScript.h"

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

    backgroundAudioBuffer = AudioBuffer::Create("sounds/test.ogg");

    loadEntities();
}

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
    mainCamera.setComponent<AudioSourceComponent>(
            AudioSource::Create(mainCameraTransform.position, false),
            mainCameraTransform.position);
    mainCamera.get<AudioSourceComponent>().source.setBuffer(backgroundAudioBuffer);

    const glm::vec4 whiteColor(1, 1, 1, 1);

    auto background = createEntity();
    background.setComponent<Meta>(Meta{"Background"});
    background.setComponent<Transform>(
            Transform{glm::vec3(0, 0, 0), glm::vec3(), glm::vec3(20, 16, 0.1)});
    const glm::vec4 greyColor(0.66f, 0.66f, 0.70f, 1);
    background.setComponent<RenderComponent>(
            texturedMaterial.instantiate(&greyColor, sizeof(greyColor), {&assetManager->getTexture("Square")}),
            quadROB);
}

void SoundTestScene::update(float /*deltaTime*/) {
    // Close window controls
    if (window->isKeyDown(GLFW_KEY_ESCAPE) ||
        (window->isKeyDown(GLFW_KEY_Q) && window->isKeyDown(GLFW_KEY_LEFT_CONTROL))) { window->close(); }
}

static bool looping = false;

void SoundTestScene::updateImGui() {
    ImGui::NewFrame();

    ImGui::Begin("Info");
    ImGuiIO &io = ImGui::GetIO();
    // Basic info
    ImGui::Text("Frame: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Viewport Size: %f x %f", io.DisplaySize.x, io.DisplaySize.y);

    ImGui::Separator();
    auto &bgAudioSource = mainCamera.get<AudioSourceComponent>().source;
    ImGui::Text("Audio controls");
    ImGui::Columns(2, "background_ctrls");

    if (ImGui::Button("Play bg"))
        bgAudioSource.play();
    ImGui::NextColumn();
    if (ImGui::Button("Pause bg"))
        bgAudioSource.pause();
    ImGui::NextColumn();
    if (ImGui::Button("Rewind bg"))
        bgAudioSource.rewind();
    ImGui::NextColumn();
    if (ImGui::Button("Stop bg"))
        bgAudioSource.stop();
    ImGui::NextColumn();

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


    ImGui::End();
}
