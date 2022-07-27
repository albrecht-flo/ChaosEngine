#include "EditorComponentUI.h"

#include <memory>
#include <optional>

#include "Engine/src/core/utils/Logger.h"
#include "Engine/src/core/utils/STDExtensions.h"
#include "Engine/src/core/Components.h"
#include "EditorComponents.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

using namespace Editor;

const std::array<std::string, 6> EditorComponentUI::componentList =
        {"Render Component", "Camera Component", "Native Script Component", "UI Text Component",
         "UI Render Component", "UI Component"};



// ------------------------------------ Rendering Helper functions -----------------------------------------------------

void EditorComponentUI::renderMaterialUI(const RenderComponentMeta &rcMeta, ChaosEngine::Entity &entity) {
    const float indentW = 16.0f;
    ImGui::Text("Material:");
    const auto &materials = assetManager.getAllMaterials();
    if (auto materialSelection = assetSelector.render(rcMeta.materialName, materials.begin(), materials.end(),
                                                      [](const auto &e) { return e.first; },
                                                      "Material")) {
        LOG_DEBUG("TODO: Handle material selection: {}", materialSelection->c_str());
    }
    ImGui::Spacing();

    ImGui::Indent(indentW);
    ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_None;
    if (assetManager.getMaterialInfo(rcMeta.materialName).hasTintColor) {
        ImGui::LabelText("##color_label_matUI", "Color");
        if (ImGui::ColorEdit4("##Color", &(editTintColor.r), colorFlags)) {
            updateMaterialInstance(entity, editTintColor, rcMeta);
        }
    }
    if (rcMeta.textures) {
        for (const auto &tex: *rcMeta.textures) {
            ImGui::Text("%s", tex.slot.c_str());
            const auto &textures = assetManager.getAllTextures();
            if (auto textureSelection = assetSelector.render(tex.texture, textures.begin(), textures.end(),
                                                             [](const auto &e) { return e.first; }, "Texture")) {
                LOG_DEBUG("TODO: Handle texture selection: {}", textureSelection->c_str());
            }
        }
    }
    ImGui::Unindent(indentW);
}

void EditorComponentUI::renderMeshUI(const RenderComponentMeta &rcMeta) {
    ImGui::Text("Mesh:");
    const auto &meshes = assetManager.getAllMeshes();
    if (auto meshSelection = assetSelector.render(rcMeta.meshName, meshes.begin(), meshes.end(),
                                                  [](const auto &e) { return e.first; }, "Mesh")) {
        LOG_DEBUG("TODO: Handle mesh selection: {}", meshSelection->c_str());
    }
}

void EditorComponentUI::renderComponentPopupList(ChaosEngine::Entity &entity) {
    ImGui::SetKeyboardFocusHere(0);
    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (ImGui::InputText("##component_menu_onput", &componentMenuInput, inputFlags) && selectedComponent >= 0) {
        addComponentToEntity(entity, componentList[selectedComponent]);
        ImGui::CloseCurrentPopup();
    }

    const ImGuiTreeNodeFlags node_flags_leaf = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    const ImGuiTreeNodeFlags node_flags_selected = ImGuiTreeNodeFlags_Selected;
    auto componentMenuInputLower = ChaosEngine::stringToLower(componentMenuInput);
    int id = 0;
    bool first = true;
    for (const auto &comp: componentList) {
        auto compLower = ChaosEngine::stringToLower(comp);
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

// ------------------------------------ ChaosEngine::Entity Helpers ----------------------------------------------------

void
EditorComponentUI::addComponentToEntity(ChaosEngine::Entity &entity, const std::string &component) {
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
    } else if (component == "Native Script Component") {
        if (entity.has<NativeScriptComponent>()) {
            LOG_ERROR("Can not add Native Script Component twice to the same entity! "\
                    "Even if they reference different scripts!");
            return;
        }
        entity.setComponent<NativeScriptComponent>(
                NativeScriptComponent{
                        std::make_unique<ChaosEngine::NativeScript>(entity),
                        true
                });
        entity.setComponent<NativeScriptComponentMeta>(NativeScriptComponentMeta{.scriptName = std::string{}});
    } else if (component == "UI Text Component") {
        if (entity.has<UITextComponent>()) {
            LOG_ERROR("Can not add UI Text Component twice to the same entity! "\
                    "Even if they reference different scripts!");
            return;
        }
        // TODO: Switch to default assets
        entity.setComponent<UITextComponent>(UITextComponent{
                .font = *(assetManager.getFont("OpenSauceSans", ChaosEngine::FontStyle::Regular, 16.0f)),
                .style = ChaosEngine::FontStyle::Regular,
                .textColor = glm::vec4(0, 0, 0, 1),
                .text = "New Text :)",
        });


    } else if (component == "UI Render Component") {
        if (entity.has<UIRenderComponent>()) {
            LOG_ERROR("Can not add UI Render Component twice to the same entity! "\
                    "Even if they reference different scripts!");
            return;
        }
        // TODO: Switch to default assets
        glm::vec4 buttonColor{1, 1, 1, 1};
//        auto &borderTexture = assetManager.getTexture("UI/Border");
        entity.setComponent<UIRenderComponent>(UIRenderComponent{
                .materialInstance = assetManager.getMaterial("UIMaterial").instantiate(
                        &buttonColor, sizeof(buttonColor),
                        {&assetManager.getTexture("UI/Border")}),
                .mesh = assetManager.getMesh("UI/Quad"),
                .scaleOffset = glm::vec3(0, 0, 0),
        });
        entity.setComponent<RenderComponentMeta>(
                "UI/Quad", "UIMaterial",
                std::make_optional(std::vector<TextureMeta>({TextureMeta{"diffuse", "UI/Border"}})));
    } else if (component == "UI Component") {
        if (entity.has<UIComponent>()) {
            LOG_ERROR("Can not add UI Component twice to the same entity! "\
                    "Even if they reference different scripts!");
            return;
        }
        // TODO: Switch to default assets
        entity.setComponent<UIComponent>(UIComponent{.active = true});
        entity.setComponent<NativeScriptComponent>(assetManager.getScript("UI/ButtonScript", entity), true);
        entity.setComponent<NativeScriptComponentMeta>(NativeScriptComponentMeta{.scriptName="UI/ButtonScript"});
    } else {
        assert("Component not implemented in EditorComponentUI::addComponentToEntity");
    }
}

void EditorComponentUI::updateMaterialInstance(ChaosEngine::Entity &entity, glm::vec4 color,
                                               const RenderComponentMeta &rcMeta) {
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

// ------------------------------------ Component rendering ------------------------------------------------------------

void EditorComponentUI::renderMetaComponentUI(ChaosEngine::Entity &entity) {
    auto &meta = entity.get<Meta>();
    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::InputText("##meta_name", &meta.name, input_text_flags);
}

void EditorComponentUI::renderTransformComponentUI(ChaosEngine::Entity &entity) {
    auto &tc = entity.get<Transform>();
    ImGui::DragFloat3("Position", &(tc.position.x), 0.25f * dragSpeed);
    ImGui::DragFloat3("Rotation", &(tc.rotation.x), 1.0f * dragSpeed);
    ImGui::DragFloat3("Scale", &(tc.scale.x), 0.25f * dragSpeed);
}

void EditorComponentUI::renderCameraComponentUI(ChaosEngine::Entity &entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("Camera Component", flags)) {
        auto &camera = entity.get<CameraComponent>();
        ImGui::InputFloat("Field of View", &camera.fieldOfView);
        ImGui::InputFloat("Near Plane", &camera.near);
        ImGui::InputFloat("Far Plane", &camera.far);
        ImGui::Checkbox("Main Camera", &camera.mainCamera);
    }
}

void EditorComponentUI::renderRenderComponentUI(ChaosEngine::Entity &entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("Render Component", flags)) {
        auto &rcMeta = entity.get<RenderComponentMeta>();

        renderMeshUI(rcMeta);
        ImGui::Spacing();
        renderMaterialUI(rcMeta, entity);
    }

}

void EditorComponentUI::renderNativeScriptComponentUI(ChaosEngine::Entity &entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("Native Script Component", flags)) {
        auto &scriptComponent = entity.get<NativeScriptComponent>();
        auto &meta = entity.get<NativeScriptComponentMeta>();

        ImGui::Checkbox("Active", &scriptComponent.active);
        ImGui::Text("Script:");
        auto name = (meta.scriptName.empty()) ? "Empty Script" : meta.scriptName;
        const auto &scripts = assetManager.getAllScripts();
        if (auto scriptSelection = assetSelector.render(name, scripts.begin(), scripts.end(),
                                                        [](const auto &e) { return e.first; }, "Script")) {
            LOG_INFO("Script selection: {}", scriptSelection->c_str());
            entity.setComponent<NativeScriptComponent>(
                    NativeScriptComponent{assetManager.getScript(*scriptSelection, entity), false}
            );
            entity.setComponent<NativeScriptComponentMeta>(NativeScriptComponentMeta{.scriptName = *scriptSelection});
        }
    }
}

void EditorComponentUI::renderUIComponentComponentUI(ChaosEngine::Entity &entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("UI Component", flags)) {
        auto &uiC = entity.get<UIComponent>();
        ImGui::Text("Enable Mouse event");
        ImGui::Checkbox("##Active_UIC", &(uiC.active));
//        auto &tc = entity.get<Transform>();
        ImGui::Text("Auto position with text");
        ImGui::Checkbox("##ScaleWithText_UIC", &(uiComponent_ScaleWithText));

        if (uiComponent_ScaleWithText)
            ImGui::BeginDisabled();
        ImGui::DragFloat3("Position (UI)", &(uiC.offsetPosition.x), 0.25f * dragSpeed);
        if (uiComponent_ScaleWithText) {
            ImGui::EndDisabled();
            uiC.offsetPosition.x = uiC.offsetScale.x;
            uiC.offsetPosition.y = uiC.offsetScale.y;
            uiC.offsetPosition.z = uiC.offsetScale.z;
        }
        ImGui::DragFloat3("Rotation (UI)", &(uiC.offsetRotation.x), 1.0f * dragSpeed);
        ImGui::DragFloat3("Scale (UI)", &(uiC.offsetScale.x), 0.25f * dragSpeed);
    }
}

void EditorComponentUI::renderUITextComponentComponentUI(ChaosEngine::Entity &entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("UI Text Component", flags)) {
        auto &uiC = entity.get<UITextComponent>();

        ImGui::Text("Text:");
        ImGui::PushItemWidth(-1);
        ImGui::InputTextMultiline("##Text", &uiC.text);
        ImGui::PopItemWidth();
        ImGui::Text("Font:");

        ChaosEngine::FontStyle style = uiC.font->getStyle();
        auto size = uiC.font->getSize();
        auto resolution = uiC.font->getResolution();

        const auto &fonts = assetManager.getAllFonts();
        std::vector<std::string> uniqueFonts;
        for (const auto &font: fonts) {
            if (std::find(uniqueFonts.begin(), uniqueFonts.end(), font.name) == uniqueFonts.end()) {
                uniqueFonts.emplace_back(font.name);
            }
        }

        if (auto fontSelection = assetSelector.render(
                uiC.font->getName(), uniqueFonts.begin(), uniqueFonts.end(),
                [](const auto &iter) { return iter; },
                "Font")) {
            uiC.font = *assetManager.getFont(*fontSelection, style, size, resolution);
        }

        const char *const styles[] = {"Regular", "Italic", "Bold"};
        assert("Font style UI needs adjustment" && ((int) ChaosEngine::FontStyle::MAX) == 3);
        const char *previewValue = styles[(int) style];
        if (ImGui::BeginCombo("Style", previewValue)) {
            for (int i = 0; i < (int) ChaosEngine::FontStyle::MAX; ++i) {
                bool isSelected = (i == (int) style);
                if (ImGui::Selectable(styles[i], isSelected)) {
                    style = (ChaosEngine::FontStyle) i;
                    uiC.font = *assetManager.getFont(uiC.font->getName(), style, size, resolution);
                }

                // Set the initial focus when opening the combo
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();

        const auto floatInputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
        if (ImGui::InputFloat("Size", &size, 1.0f, 10.0f, "%.f", floatInputFlags)) {
            if (8.0f <= size && size <= 256.0f) {
                LOG_DEBUG("TODO Font Size adjust.");
                // uiC.font = assetManager.loadFont(uiC.font->getName(), style, size, resolution);
            }
        }
        if (ImGui::InputFloat("Resolution", &resolution, 1.0f, 10.0f, "%.f", floatInputFlags)) {
            if (64.0f <= size && size <= 512.0f) {
                LOG_DEBUG("TODO Font Resolution adjust.");
                // uiC.font = assetManager.loadFont(uiC.font->getName(), style, size, resolution);
            }
        }

        ImGui::Spacing();

        ImGui::ColorEdit4("Text Color", &(uiC.textColor.r));
    }
}


void EditorComponentUI::renderUIRenderComponentComponentUI(ChaosEngine::Entity &entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("UI Render Component", flags)) {
        auto &rcMeta = entity.get<RenderComponentMeta>();

        renderMeshUI(rcMeta);
        ImGui::Spacing();
        renderMaterialUI(rcMeta, entity);
        ImGui::Spacing();

        UIRenderComponent &uiRC = entity.get<UIRenderComponent>();
        ImGui::Text("Scale Offset:");
        ImGui::DragFloat3("##Scale_Offset", &(uiRC.scaleOffset.x), 0.25f * dragSpeed);
    }
}

// ------------------------------------ Core rendering function --------------------------------------------------------

bool EditorComponentUI::renderEntityComponentPanel(ChaosEngine::Entity &entity) {
    const auto panelWidth = ImGui::GetContentRegionAvailWidth();

    if (entity.has<Meta>()) {
        renderMetaComponentUI(entity);
    } else {
        LOG_ERROR("All entities selectable by the editor MUST have a Meta component");
        ImGui::TextColored(ImVec4{1, 0, 0, 1}, "ERROR !!!\n----------------");
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + panelWidth);
        ImGui::TextColored(ImVec4{1, 0, 0, 1}, "All entities selectable by the editor MUST have a Meta component");
        ImGui::PopTextWrapPos();
        return false;
    }

    ImGui::SameLine(panelWidth - 60);
    bool deleted = ImGui::Button("Delete");
    ImGui::Separator();
    ImGui::Spacing();
    if (deleted)
        return true;

    if (entity.has<Transform>()) {
        renderTransformComponentUI(entity);
        ImGui::Separator();
        ImGui::Spacing();
    } else {
        LOG_WARN("Entity '{}' has not transform, are you sure this is right?", entity.get<Meta>().name);
    }

    if (entity.has<CameraComponent>()) {
        renderCameraComponentUI(entity);
        ImGui::Separator();
        ImGui::Spacing();
    }

    if (entity.has<RenderComponent>()) {
        renderRenderComponentUI(entity);
        ImGui::Separator();
        ImGui::Spacing();
    } else if (entity.has<UIRenderComponent>()) {
        renderUIRenderComponentComponentUI(entity);
        ImGui::Separator();
        ImGui::Spacing();
    }


    if (entity.has<NativeScriptComponent>()) {
        renderNativeScriptComponentUI(entity);
        ImGui::Separator();
        ImGui::Spacing();
    }

    if (entity.has<UITextComponent>()) {
        renderUITextComponentComponentUI(entity);
        ImGui::Separator();
        ImGui::Spacing();
    }
    if (entity.has<UIComponent>()) {
        renderUIComponentComponentUI(entity);
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
