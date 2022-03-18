#include "EditorComponentUI.h"

#include "Engine/src/core/Utils/Logger.h"
#include "Engine/src/core/Components.h"
#include "EditorComponents.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

using namespace Editor;

static std::string stringToLower(const std::string &str) {
    std::string copy(str);
    std::transform(copy.begin(), copy.end(), copy.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return copy;
}


const std::array<std::string, 2> EditorComponentUI::componentList = {"Render Component", "Camera Component"};

// ------------------------------------ Component rendering ------------------------------------------------------------

void EditorComponentUI::renderMetaComponentUI(Entity &entity) {
    auto &meta = entity.get<Meta>();
    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::InputText("##meta_name", &meta.name, input_text_flags);
}

void EditorComponentUI::renderTransformComponentUI(Entity &entity) {
    auto &tc = entity.get<Transform>();
    ImGui::DragFloat3("Position", &(tc.position.x), 0.25f * dragSpeed);
    ImGui::DragFloat3("Rotation", &(tc.rotation.x), 1.0f * dragSpeed);
    ImGui::DragFloat3("Scale", &(tc.scale.x), 0.25f * dragSpeed);
}

void EditorComponentUI::renderCameraComponentUI(Entity &entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("Camera Component", flags)) {
        auto &camera = entity.get<CameraComponent>();
        ImGui::InputFloat("Field of View", &camera.fieldOfView);
        ImGui::InputFloat("Near Plane", &camera.near);
        ImGui::InputFloat("Far Plane", &camera.far);
        ImGui::Checkbox("Main Camera", &camera.mainCamera);
    }
}

void EditorComponentUI::renderRenderComponentUI(Entity &entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("Render Component", flags)) {
        auto &rcMeta = entity.get<RenderComponentMeta>();
        const auto panelWidth = ImGui::GetContentRegionAvailWidth();
        const float indentW = 16.0f;

        ImGui::Text("Mesh:");
        if (ImGui::Button(rcMeta.meshName.c_str(), ImVec2(panelWidth, 0))) {
            // TODO
        }
        ImGui::Spacing();

        ImGui::Text("Material:");
        if (ImGui::Button(rcMeta.materialName.c_str(), ImVec2(panelWidth, 0))) {
            // TODO
        }
        ImGui::Spacing();

        ImGui::Indent(indentW);
        ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_None;
        if (assetManager.getMaterialInfo(rcMeta.materialName).hasTintColor) {
            if (ImGui::ColorEdit4("Color", &(editTintColor.r), colorFlags)) {
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

bool EditorComponentUI::renderEntityComponentPanel(Entity &entity) {
    renderMetaComponentUI(entity);

    const auto panelWidth = ImGui::GetContentRegionAvailWidth();
    ImGui::SameLine(panelWidth - 60);
    bool deleted = ImGui::Button("Delete");
    ImGui::Separator();
    ImGui::Spacing();
    if (deleted)
        return true;

    renderTransformComponentUI(entity);
    ImGui::Separator();
    ImGui::Spacing();

    if (entity.has<CameraComponent>()) {
        renderCameraComponentUI(entity);
        ImGui::Separator();
        ImGui::Spacing();
    }

    if (entity.has<RenderComponent>()) {
        renderRenderComponentUI(entity);
        ImGui::Separator();
        ImGui::Spacing();
    }

    ImGui::NewLine();
    if (ImGui::Button("Add Component", ImVec2(panelWidth, 0))) {
        ImGui::OpenPopup("##add_component_popup");
        componentMenuInput = "";
        ImVec2 cursor = ImGui::GetCurrentContext()->IO.MousePos;
        ImGui::SetNextWindowPos(ImVec2(cursor.x - 128, cursor.y));
    }
    ImGuiWindowFlags popupFlags = ImGuiWindowFlags_Popup;
    if (ImGui::BeginPopup("##add_component_popup", popupFlags)) {
        renderComponentPopupList(entity);
        ImGui::EndPopup();
    }

    return false;
}

void EditorComponentUI::renderComponentPopupList(Entity &entity) {
    ImGui::SetKeyboardFocusHere(0);
    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (ImGui::InputText("##component_menu_onput", &componentMenuInput, inputFlags) && selectedComponent >= 0) {
        addComponentToEntity(entity, componentList[selectedComponent]);
        ImGui::CloseCurrentPopup();
    }

    const ImGuiTreeNodeFlags node_flags_leaf = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    const ImGuiTreeNodeFlags node_flags_selected = ImGuiTreeNodeFlags_Selected;
    auto componentMenuInputLower = stringToLower(componentMenuInput);
    int id = 0;
    bool first = true;
    for (const auto &comp: componentList) {
        auto compLower = stringToLower(comp);
        if (componentMenuInput.empty() || compLower.find(componentMenuInputLower) != std::string::npos) {
            if (first) {
                selectedComponent = id;
                first = false;
            }
            ImGui::TreeNodeEx(comp.c_str(), node_flags_leaf | ((selectedComponent == id) ? node_flags_selected : 0));
            if (ImGui::IsItemClicked()) {
                selectedComponent = id;
                addComponentToEntity(entity, comp);
                ImGui::CloseCurrentPopup();
            }
        }
        id++;
    }
    if (first) {
        selectedComponent = -1;
    }
}

// ------------------------------------ Entity Helpers -----------------------------------------------------------------
void EditorComponentUI::addComponentToEntity(Entity &entity, const std::string &component) {
    LOG_DEBUG("Adding Component {}", component.c_str());
    if (component == "Render Component") {
        if (entity.has<RenderComponent>()) {
            LOG_ERROR("Can not add Render Component twice to the same entity!");
            return;
        }
        glm::vec4 whiteTintColor(1, 1, 1, 1);
        auto[mesh, meshName] = editorAssets.getDefaultMesh();
        auto[material, materialInfo] = editorAssets.getDefaultMaterial();
        auto[textureSet, textureSetInfo] = editorAssets.getDefaultTextureSet();
        entity.setComponent<RenderComponent>(
                material.instantiate(&whiteTintColor, sizeof(whiteTintColor), textureSet),
                mesh
        );
        entity.setComponent<RenderComponentMeta>(meshName, materialInfo, std::make_optional(textureSetInfo));
    } else if (component == "Camera Component") {
        if (entity.has<CameraComponent>()) {
            LOG_ERROR("Can not add Camera Component twice to the same entity!");
            return;
        }
        entity.setComponent<CameraComponent>(CameraComponent{
                .fieldOfView = 45.0f,
                .near = 0.1f,
                .far = 100.0f,
                .active = false,
                .mainCamera = false,
        });
    } else {
        assert("Component not implemented in EditorComponentUI::addComponentToEntity");
    }
}

void EditorComponentUI::updateMaterialInstance(Entity &entity, glm::vec4 color, const RenderComponentMeta &rcMeta) {
    auto mesh = assetManager.getMesh(rcMeta.meshName);
    auto material = assetManager.getMaterial(rcMeta.materialName);

    std::vector<const Renderer::Texture *> textureSet{};
    if (rcMeta.textures) {
        for (const auto &texMeta: *rcMeta.textures) {
            textureSet.emplace_back(&assetManager.getTexture(texMeta.texture));
        }
    }

    auto mat = material.instantiate(&color, sizeof(color), textureSet);
    entity.setComponent<RenderComponent>(mat, mesh);
}
