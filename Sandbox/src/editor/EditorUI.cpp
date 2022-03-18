#include "EditorUI.h"

#include "Engine/src/core/Components.h"
#include "EditorComponents.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace Editor;

// ------------------------------------ Component rendering ------------------------------------------------------------

void EditorUI::renderMetaComponentUI(Entity &entity) {
    auto &meta = entity.get<Meta>();
    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::InputText(" ", &meta.name, input_text_flags);
}

void EditorUI::renderTransformComponentUI(Entity &entity) {
    auto &tc = entity.get<Transform>();
    ImGui::DragFloat3("Position", &(tc.position.x), 0.25f * dragSpeed);
    ImGui::DragFloat3("Rotation", &(tc.rotation.x), 1.0f * dragSpeed);
    ImGui::DragFloat3("Scale", &(tc.scale.x), 0.25f * dragSpeed);
}

void EditorUI::renderCameraComponentUI(Entity &entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("Camera Component", flags)) {
        auto &camera = entity.get<CameraComponent>();
        ImGui::InputFloat("Field of View", &camera.fieldOfView);
        ImGui::InputFloat("Near Plane", &camera.near);
        ImGui::InputFloat("Far Plane", &camera.far);
        ImGui::Checkbox("Main Camera", &camera.mainCamera);
    }
}

void EditorUI::renderRenderComponentUI(Entity &entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("Render Component", flags)) {
        auto &rcMeta = entity.get<RenderComponentMeta>();
        const auto panelWidth = ImGui::GetContentRegionAvailWidth();
        const float indentW = 16.0f;

        ImGui::Text("Mesh:");
        if (ImGui::Button(rcMeta.meshName.c_str(), ImVec2(panelWidth, 0))) {
            // TODO
        }
        ImGui::Text("Material:");
        if (ImGui::Button(rcMeta.materialName.c_str(), ImVec2(panelWidth, 0))) {
            // TODO
        }
        ImGui::Indent(indentW);
        if (assetManager.getMaterialInfo(rcMeta.materialName).hasTintColor) {
            if (ImGui::ColorEdit4("Color", &(editTintColor.r))) {
                updateMaterialInstance(entity, editTintColor, rcMeta);
            }
        }
        if (rcMeta.textures) {
            for (const auto &tex: *rcMeta.textures) {
                ImGui::Text("%s", tex.slot.c_str());
                if (ImGui::Button(tex.texture.c_str(), ImVec2(panelWidth, 0))) {
                    // TODO
                }
            }
        }
        ImGui::Unindent(indentW);
    }
}

// ------------------------------------ Class Members ------------------------------------------------------------------

bool EditorUI::renderEntityComponentPanel(Entity &entity) {
    renderMetaComponentUI(entity);

    const auto panelWidth = ImGui::GetWindowWidth();
    ImGui::SameLine(panelWidth - 60);
    bool deleted = ImGui::Button("Delete");
    ImGui::Separator();
    if (deleted)
        return true;

    renderTransformComponentUI(entity);
    ImGui::Separator();

    if (entity.has<CameraComponent>()) {
        renderCameraComponentUI(entity);
        ImGui::Separator();
    }

    if (entity.has<RenderComponent>()) {
        renderRenderComponentUI(entity);
        ImGui::Separator();
    }

    return false;
}

void EditorUI::updateMaterialInstance(Entity &entity, glm::vec4 color, const RenderComponentMeta &rcMeta) {
    // TODO
//    auto mesh = assets.getMesh(rcMeta);
//    auto& material = assets.getMaterial(rcMeta);
//    auto textureSet = assets.getTextureSet(rcMeta);
//    entity.setComponent<RenderComponent>(
//            material.instantiate(&color, sizeof(color),textureSet),
//            mesh);
}
