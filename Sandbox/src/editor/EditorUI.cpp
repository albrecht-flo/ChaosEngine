#include "EditorUI.h"

#include "Engine/src/core/Components.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace Editor;

EditorUI::EditorUIState EditorUI::state = EditorUI::EditorUIState{
        .editTintColor = glm::vec4(1, 1, 1, 1),
        .dragSpeed = 1,
};

// ------------------------------------ Component rendering ------------------------------------------------------------

void EditorUI::renderMetaComponentUI(Entity &entity) {
    auto &meta = entity.get<Meta>();
    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::InputText(" ", &meta.name, input_text_flags);
}

void EditorUI::renderTransformComponentUI(Entity &entity) {
    auto &tc = entity.get<Transform>();
    ImGui::DragFloat3("Position", &(tc.position.x), 0.25f * state.dragSpeed);
    ImGui::DragFloat3("Rotation", &(tc.rotation.x), 1.0f * state.dragSpeed);
    ImGui::DragFloat3("Scale", &(tc.scale.x), 0.25f * state.dragSpeed);
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

void EditorUI::renderRenderComponentUI(Entity &entity, const EditorBaseAssets &assets) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("Render Component", flags)) {
        ImGui::ColorEdit4("Color", &(state.editTintColor.r));
        if (ImGui::Button("Apply")) {
            entity.setComponent<RenderComponent>(
                    assets.getTexturedMaterial().instantiate(&state.editTintColor, sizeof(state.editTintColor),
                                                             {&assets.getFallbackTexture()}),
                    assets.getQuadMesh());
        }
    }
}

// ------------------------------------ Class Members ------------------------------------------------------------------

bool EditorUI::renderEntityComponentPanel(Entity &entity, const EditorBaseAssets &assets) {
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
        renderRenderComponentUI(entity, assets);
        ImGui::Separator();
    }

    return false;
}
