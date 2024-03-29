#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <functional>

#include "Engine/src/core/assets/AssetManager.h"
#include "Engine/src/core/utils/Logger.h"
#include "Engine/src/core/utils/STDExtensions.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

namespace Editor {

    class EditorAssetSelector {
    public:
        explicit EditorAssetSelector(const ChaosEngine::AssetManager &assetManager) : assetManager(assetManager) {}

        ~EditorAssetSelector() = default;

        template<class Iter>
        std::optional<std::string>
        render(const std::string &label, Iter begin, Iter end,
               std::function<std::string(const typename Iter::value_type &)> getStringOfItem,
               const std::string &type) {
            std::optional<std::string> result = std::nullopt;
            const std::string imGUIId = std::string("##") + label;
            const auto panelWidth = ImGui::GetContentRegionAvailWidth();
            if (ImGui::Button(label.c_str(), ImVec2(panelWidth, 0))) {
                LOG_DEBUG("Button pressed {}", label.c_str());
                ImGui::OpenPopup(imGUIId.c_str());
                currentSelection = "";
                assetFilterInput = "";
                ImVec2 windowPos = ImGui::GetCurrentContext()->CurrentViewport->Pos;
                ImVec2 windowSize = ImGui::GetCurrentContext()->CurrentViewport->Size;
                ImGui::SetNextWindowPos(ImVec2(windowPos.x + windowSize.x / 2 - 50, windowPos.y + 64));
            }

            if (ImGui::BeginPopup(imGUIId.c_str())) {
                const std::string popupTitle = std::string("Asset selector -- Type: ") + type;
                ImGui::Text(popupTitle.c_str());
                ImGui::Separator();
                ImGui::SetKeyboardFocusHere(0);
                ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
                if (ImGui::InputText("##component_menu_onput", &assetFilterInput, inputFlags) &&
                    !currentSelection.empty()) {
                    LOG_DEBUG("Selected asset: {}", currentSelection.c_str());
                    result = std::make_optional(currentSelection);
                    ImGui::CloseCurrentPopup();
                }

                const ImGuiTreeNodeFlags node_flags_leaf =
                        ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                const ImGuiTreeNodeFlags node_flags_selected = ImGuiTreeNodeFlags_Selected;
                bool first = true;
                auto assetFilterInputLower = ChaosEngine::stringToLower(assetFilterInput);
                for (Iter itr = begin; itr != end; ++itr) {
                    const std::string &assetUri = getStringOfItem(*itr);
                    auto assetUriLower = ChaosEngine::stringToLower(assetUri);
                    if (currentSelection.empty() || assetUriLower.find(assetFilterInputLower) != std::string::npos) {
                        if (first) {
                            currentSelection = assetUri;
                            first = false;
                        }
                        ImGui::TreeNodeEx(assetUri.c_str(),
                                          node_flags_leaf | ((currentSelection == assetUri) ? node_flags_selected : 0));
                        if (ImGui::IsItemClicked()) {
                            currentSelection = assetUri;
                            LOG_DEBUG("Selected asset: {}", currentSelection.c_str());
                            result = std::make_optional(currentSelection);
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
                if (first) {
                    currentSelection = "";
                }
                ImGui::EndPopup();
            }

            return result;
        }

    private:
        const ChaosEngine::AssetManager &assetManager;
        std::string currentSelection{};
        std::string assetFilterInput{};
    };

}

