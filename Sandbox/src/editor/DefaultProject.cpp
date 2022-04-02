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
                    Transform{glm::vec3{64, 256, -1}, glm::vec3(0, 0, 33), glm::vec3(1, 1, 1)});
            textTester.setComponent<UITextComponent>(UITextComponent{
                    .font = *(assetManager.getFont("OpenSauceSans", FontStyle::Regular)),
                    .style = FontStyle::Regular,
                    .textColor = glm::vec4(0.3f, 0, 0.3f, 1),
                    .text = "This is some Text with,\nmore than 1 line :)\nAnd Special Characters xD\n!@#$%^&*()-_=+[]{}'\":;,.<>/?",
            });
            auto textTesterI = scene.createEntity();
            textTesterI.setComponent<Meta>("Text Tester Italic");
            textTesterI.setComponent<Transform>(
                    Transform{glm::vec3{64, 432, -1}, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
            textTesterI.setComponent<UITextComponent>(UITextComponent{
                    .font = *(assetManager.getFont("OpenSauceSans", FontStyle::Italic)),
                    .style = FontStyle::Regular,
                    .textColor = glm::vec4(0.3f, 0, 0.3f, 1),
                    .text = "This is some italic Text about some quick brown foxes xD",
            });
            auto textTesterB = scene.createEntity();
            textTesterB.setComponent<Meta>("Text Tester Bold");
            textTesterB.setComponent<Transform>(
                    Transform{glm::vec3{64, 500, -1}, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
            textTesterB.setComponent<UITextComponent>(UITextComponent{
                    .font = *(assetManager.getFont("OpenSauceSans", FontStyle::Bold)),
                    .style = FontStyle::Regular,
                    .textColor = glm::vec4(0.3f, 0, 0.3f, 1),
                    .text = "This is some bold Text about a lazy dog xD",
            });
        }
    }
}