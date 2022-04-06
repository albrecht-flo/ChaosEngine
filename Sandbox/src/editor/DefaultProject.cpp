#include "DefaultProject.h"

#include "core/utils/Logger.h"
#include "EditorComponents.h"

using namespace ChaosEngine;

namespace Editor {

    void loadDefaultSceneEntities(ChaosEngine::Scene &scene, EditorBaseAssets &assets,
                                  ChaosEngine::AssetManager &assetManager) {
        LOG_INFO("Loading entities");

        auto camera = scene.createEntity();
        camera.setComponent<Meta>(Meta{"Main Camera"});
        camera.setComponent<Transform>(Transform{glm::vec3(0, 0, -2), glm::vec3(), glm::vec3(1, 1, 1)});
        camera.setComponent<CameraComponent>(
                CameraComponent{.fieldOfView=10.0f, .near=0.1f, .far=100.0f, .active=false, .mainCamera = true});

        auto texturedQuad = scene.createEntity();
        texturedQuad.setComponent<Meta>(Meta{"Textured quad"});
        texturedQuad.setComponent<Transform>(Transform{glm::vec3(2, -2, 0), glm::vec3(0, 0, 45), glm::vec3(1, 1, 1)});
        glm::vec4 whiteTintColor(1, 1, 1, 1);
        texturedQuad.setComponent<RenderComponent>(
                assets.getTexturedMaterial().instantiate(
                        &whiteTintColor, sizeof(whiteTintColor), {&assets.getFallbackTexture()}),
                assets.getQuadMesh()
        );
        texturedQuad.setComponent<RenderComponentMeta>(assets.getQuadMeshName(),
                                                       assets.getTexturedMaterial()->getName(),
                                                       std::make_optional(std::vector<TextureMeta>(
                                                               {TextureMeta{"diffuse",
                                                                            assets.getFallbackTextureName()}})));

        auto hexagon = scene.createEntity();
        hexagon.setComponent<Meta>(Meta{"Textured hexagon"});
        hexagon.setComponent<Transform>(Transform{glm::vec3(-2, -2, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
        hexagon.setComponent<RenderComponent>(
                assets.getTexturedMaterial().instantiate(
                        &whiteTintColor, sizeof(whiteTintColor), {&assets.getFallbackTexture()}),
                assets.getHexMesh()
        );
        hexagon.setComponent<RenderComponentMeta>(assets.getHexMeshName(),
                                                  assets.getTexturedMaterial()->getName(),
                                                  std::make_optional(std::vector<TextureMeta>(
                                                          {TextureMeta{"diffuse", assets.getFallbackTextureName()}})));

        auto hexagonD = scene.createEntity();
        hexagonD.setComponent<Meta>(Meta{"Debug hexagon"});
        hexagonD.setComponent<Transform>(Transform{glm::vec3(0, 3, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
        glm::vec4 blueColor(0, 0, 1, 1);
        hexagonD.setComponent<RenderComponent>(
                assets.getDebugMaterial().instantiate(&blueColor, sizeof(blueColor), {}),
                assets.getHexMesh()
        );
        hexagonD.setComponent<RenderComponentMeta>(assets.getHexMeshName(), assets.getDebugMaterial()->getName(),
                                                   std::nullopt);

        { // UI elements
            auto textTester = scene.createEntity();
            textTester.setComponent<Meta>("Text Tester Multiline");
            textTester.setComponent<Transform>(
                    Transform{glm::vec3{16, 126, 2}, glm::vec3(0, 0, 33), glm::vec3(1, 1, 1)});
            textTester.setComponent<UITextComponent>(UITextComponent{
                    .font = *(assetManager.getFont("OpenSauceSans", FontStyle::Regular, 16.0f, 95.0f)),
                    .style = FontStyle::Regular,
                    .textColor = glm::vec4(0.3f, 0, 0.3f, 1),
                    .text = "This is some Text with,\nmore than 1 line :)\nAnd Special Characters xD\n!@#$%^&*()-_=+[]{}'\":;,.<>/?",
            });
            auto textTesterI = scene.createEntity();
            textTesterI.setComponent<Meta>("Text Tester Italic");
            textTesterI.setComponent<Transform>(
                    Transform{glm::vec3{16, 210, 2}, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
            textTesterI.setComponent<UITextComponent>(UITextComponent{
                    .font = *(assetManager.getFont("OpenSauceSans", FontStyle::Italic, 16.0f, 95.0f)),
                    .style = FontStyle::Italic,
                    .textColor = glm::vec4(1.0f, 1, 1.0f, 1),
                    .text = "This is some italic Text about some quick brown foxes xD",
            });
            auto textTesterB = scene.createEntity();
            textTesterB.setComponent<Meta>("Text Tester Bold");
            textTesterB.setComponent<Transform>(
                    Transform{glm::vec3{16, 240, 2}, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
            textTesterB.setComponent<UITextComponent>(UITextComponent{
                    .font = *(assetManager.getFont("OpenSauceSans", FontStyle::Bold, 16.0f, 95.0f)),
                    .style = FontStyle::Bold,
                    .textColor = glm::vec4(0.66f, 0, 0.0f, 1),
                    .text = "This is some bold Text about a lazy dog xD (in red)",
            });

            const std::string uiMeshName = "UI/Quad";
            auto uiMesh = assetManager.getMesh(uiMeshName);
            auto uiMaterial = assetManager.getMaterial("UIMaterial");
            auto &borderTexture = assetManager.getTexture("UI/Border");

            glm::vec4 buttonColor0{1, 0.5f, 0.5f, 1};
            auto button0 = scene.createEntity();
            button0.setComponent<Meta>("Test Button with 8x4 pixel offset rendering");
            button0.setComponent<Transform>(
                    Transform{glm::vec3{512, 56, 1}, glm::vec3(0, 0, 0), glm::vec3(64, 25, 1)});
            button0.setComponent<UIRenderComponent>(UIRenderComponent{
                    .materialInstance = uiMaterial.instantiate(&buttonColor0, sizeof(buttonColor0), {&borderTexture}),
                    .mesh = uiMesh,
                    .scaleOffset = glm::vec3(8, 4, 0),
            });
            button0.setComponent<UIComponent>(UIComponent{.active = true});
            button0.setComponent<NativeScriptComponent>(assetManager.getScript("UI/ButtonScript", button0), true);
            button0.setComponent<NativeScriptComponentMeta>(NativeScriptComponentMeta{.scriptName="UI/ButtonScript"});
            button0.setComponent<RenderComponentMeta>(
                    uiMeshName, uiMaterial->getName(),
                    std::make_optional(std::vector<TextureMeta>({TextureMeta{"diffuse", "UI/Border"}})));

            glm::vec4 buttonColor1{0.5f, 0.5f, 1.0f, 1};
            auto button1 = scene.createEntity();
            button1.setComponent<Meta>("Test Button with rotation");
            button1.setComponent<Transform>(
                    Transform{glm::vec3{650, 128, 1}, glm::vec3(0, 0, 45.0f), glm::vec3(1, 1, 1)});
            button1.setComponent<UIRenderComponent>(UIRenderComponent{
                    .materialInstance = uiMaterial.instantiate(&buttonColor1, sizeof(buttonColor1), {&borderTexture}),
                    .mesh = uiMesh,
                    .scaleOffset = glm::vec3(0, 0, 0),
            });
            button1.setComponent<UIComponent>(
                    UIComponent{.active = true, .offsetPosition={60, -30, -0.1f}, .offsetRotation={0, 0, 0},
                            .offsetScale={75, 22, 0}});
            button1.setComponent<NativeScriptComponent>(assetManager.getScript("UI/ButtonScript", button1), true);
            button1.setComponent<NativeScriptComponentMeta>(NativeScriptComponentMeta{.scriptName="UI/ButtonScript"});
            button1.setComponent<RenderComponentMeta>(
                    uiMeshName, uiMaterial->getName(),
                    std::make_optional(std::vector<TextureMeta>({TextureMeta{"diffuse", "UI/Border"}})));
            button1.setComponent<UITextComponent>(UITextComponent{
                    .font = assetManager.loadFont("OpenSauceSans", "fonts/OpenSauceSans-Bold.ttf", FontStyle::Bold,
                                                  18.0f, 95.0f),
                    .style = FontStyle::Bold,
                    .textColor = glm::vec4(0.33f, 0, 0.1f, 1),
                    .text = "Click me :D",
            });


            auto &oBorderTexture = assetManager.getTexture("UI/OBorder");
            glm::vec4 buttonColor2{0.1f, 0.0f, 0.7f, 1};
            auto button2 = scene.createEntity();
            button2.setComponent<Meta>("Opaque Test Button");
            button2.setComponent<Transform>(
                    Transform{glm::vec3{120, 400, 1}, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
            button2.setComponent<UIRenderComponent>(UIRenderComponent{
                    .materialInstance = uiMaterial.instantiate(&buttonColor1, sizeof(buttonColor1), {&oBorderTexture}),
                    .mesh = uiMesh,
                    .scaleOffset = glm::vec3(0, 0, 0),
            });
            button2.setComponent<UIComponent>(
                    UIComponent{.active = true, .offsetPosition={35, 21, -0.1f}, .offsetRotation={0, 0, 0},
                            .offsetScale={50, 20, 0}});
            button2.setComponent<NativeScriptComponent>(assetManager.getScript("UI/ButtonScript", button2), true);
            button2.setComponent<NativeScriptComponentMeta>(NativeScriptComponentMeta{.scriptName="UI/ButtonScript"});
            button2.setComponent<RenderComponentMeta>(
                    uiMeshName, uiMaterial->getName(),
                    std::make_optional(std::vector<TextureMeta>({TextureMeta{"diffuse", "UI/Border"}})));
            button2.setComponent<UITextComponent>(UITextComponent{
                    .font = assetManager.loadFont("OpenSauceSans", "fonts/OpenSauceSans-Bold.ttf", FontStyle::Bold,
                                                  18.0f, 95.0f),
                    .style = FontStyle::Bold,
                    .textColor = glm::vec4(0.0f, 0, 0.0f, 1),
                    .text = "Button",
            });
        }
    }
}