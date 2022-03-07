#pragma once

#include "Engine/src/renderer/api/Framebuffer.h"
#include "Engine/src/core/Scene.h"

#include <imgui.h>

#include <functional>
#include <string>

class Window;
namespace CustomImGui {

    class CoreImGui {
    private:
        struct CoreImGuiState {
            // Log data
            bool followLog = true;
            // Viewport data
            ImVec2 previousSize = ImVec2(0.0f, 0.0f);
            std::vector<void *> sceneImageGPUHandles;
            uint32_t currentFrame = 0;
        };

    public:
        static void ImGuiEnableDocking(const std::function<void(void)> &menuCallback);

        static void RenderLogWindow(const std::string &title = "Log");

        static ImVec2
        RenderSceneViewport(const Renderer::Framebuffer &framebuffer, const std::string &title = "Viewport",
                            bool *focused = nullptr);

        // Trees -----------------------------------------------------------------------------------------------------------
        // See ImGui Issue: https://github.com/ocornut/imgui/issues/581
        // And demo: https://github.com/ocornut/imgui/commit/ac501102fc7524433c2343f2fa2cc47ea08f548e
    private:
        static const ImGuiTreeNodeFlags node_flags_base =
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        static const ImGuiTreeNodeFlags node_flags_leaf =
                node_flags_base | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        static const ImGuiTreeNodeFlags node_flags_selected = ImGuiTreeNodeFlags_Selected;
    public:

        template<class id_type, class... Args>
        static bool TreeLeaf(id_type id, id_type *id_ptr, const char *fmt, Args &&... args) {
            bool nodeState = ImGui::TreeNodeEx((void *) (intptr_t) id,
                                               node_flags_leaf | (*id_ptr == id ? node_flags_selected : 0),
                                               fmt, std::forward<Args>(args)...);
            if (ImGui::IsItemClicked() && *id_ptr != id) {
                *id_ptr = id;
            }
            return nodeState;
        }

        template<class id_type, class... Args>
        static bool TreeNodeBegin(id_type id, id_type *id_ptr, const char *fmt, Args &&... args) {
            bool nodeState = ImGui::TreeNodeEx((void *) (intptr_t) id,
                                               node_flags_base | (*id_ptr == id ? node_flags_selected : 0),
                                               fmt, std::forward<Args>(args)...);
            if (ImGui::IsItemClicked() && *id_ptr != id) {
                *id_ptr = id;
            }
            return nodeState;
        }

        static void TreeNodeEnd() { ImGui::TreePop(); }

    private:
        static CoreImGuiState state;
    };

}