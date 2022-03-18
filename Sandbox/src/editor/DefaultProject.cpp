#include "DefaultProject.h"

#include "core/Utils/Logger.h"
#include "EditorComponents.h"

namespace Editor {
    void loadDefaultSceneEntities(Scene &scene, EditorBaseAssets &assets) {
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
                assets.getQuadMesh(), assets.TexturedMaterialID
        );
        texturedQuad.setComponent<RenderComponentMeta>(assets.getTexturedMaterial()->getName(),
                                                       assets.getQuadMeshName(),
                                                       std::make_optional(std::vector<TextureMeta>(
                                                               {TextureMeta{"diffuse", "fallback-0.tex"}})),
                                                       assets.TexturedMaterialID);

        auto hexagon = scene.createEntity();
        hexagon.setComponent<Meta>(Meta{"Textured hexagon"});
        hexagon.setComponent<Transform>(Transform{glm::vec3(-2, -2, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
        hexagon.setComponent<RenderComponent>(
                assets.getTexturedMaterial().instantiate(
                        &whiteTintColor, sizeof(whiteTintColor), {&assets.getFallbackTexture()}),
                assets.getHexMesh(), assets.TexturedMaterialID
        );
        hexagon.setComponent<RenderComponentMeta>(assets.getTexturedMaterial()->getName(),
                                                  assets.getHexMeshName(),
                                                  std::make_optional(std::vector<TextureMeta>(
                                                          {TextureMeta{"diffuse", "fallback-0.tex"}})),
                                                  assets.TexturedMaterialID);

        auto hexagonD = scene.createEntity();
        hexagonD.setComponent<Meta>(Meta{"Debug hexagon"});
        hexagonD.setComponent<Transform>(Transform{glm::vec3(0, 3, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)});
        glm::vec4 blueColor(0, 0, 1, 1);
        hexagonD.setComponent<RenderComponent>(
                assets.getDebugMaterial().instantiate(&blueColor, sizeof(blueColor), {}),
                assets.getHexMesh(), assets.DebugMaterialID
        );
        hexagonD.setComponent<RenderComponentMeta>(assets.getDebugMaterial()->getName(), assets.getHexMeshName(),
                                                   std::nullopt, assets.DebugMaterialID);
    }
}