#include "CustomImGui.h"

#include "ChaosEngine.h"
#include "Engine/src/renderer/vulkan/image/VulkanTexture.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include <vector>
#include <string>

CustomImGui::CustomImGuiState CustomImGui::state{};

// Source: https://gist.github.com/Pikachuxxxx/a3796bb193ca0aaed4ad4f591b2dab07
void CustomImGui::ImGuiEnableDocking(const std::function<void(void)> &menuCallback) {
    static bool opt_fullscreen = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    } else {
        Logger::E("ImGui", "Cannot Dock Windows");
    }

    menuCallback();

    ImGui::End();
}

void CustomImGui::RenderLogWindow(const std::string &title) {
    ImGui::Begin(title.c_str());
    if (ImGui::Button("Toggle auto scroll"))
        state.followLog = !state.followLog;
    std::vector<std::string> logger = Logger::GetLogBuffer();
    if (ImGui::BeginListBox("##LogList", ImVec2{-FLT_MIN, -FLT_MIN})) {
        for (const auto &str: logger) {
            ImGui::Selectable(str.c_str());
        }

        auto left_up = ImGui::GetWindowPos();
        auto right_down = ImVec2{left_up.x + ImGui::GetWindowWidth(), left_up.y + ImGui::GetWindowHeight()};
        if (ImGui::IsMouseHoveringRect(left_up, right_down) && ImGui::GetIO().MouseWheel != 0)
            state.followLog = false;
        if (ImGui::IsMouseHoveringRect(left_up, right_down) && ImGui::GetIO().MouseWheel < 0 &&
            ImGui::GetScrollY() == ImGui::GetScrollMaxY())
            state.followLog = true;

        if (state.followLog)
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
        ImGui::EndListBox();
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to create log list!");
    }
    ImGui::End();
}


ImVec2 CustomImGui::renderViewport(const Renderer::Framebuffer &framebuffer) {
    // NEXT Rework ImGui Texture allocation (update)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport");
    auto size = ImGui::GetContentRegionAvail();
    if (state.previousSize.x != size.x || state.previousSize.y != size.y) {
        state.previousSize = size;
        RenderingSystem::GetCurrentRenderer().requestViewportResize(glm::vec2(size.x, size.y));
    }
    const auto &tex = dynamic_cast<const VulkanTexture &>(framebuffer.getAttachmentTexture(
            Renderer::AttachmentType::Color, 0));
    // TODO Reuse (Currently this breaks after ~ 1000 allocations
    auto x = ImGui_ImplVulkan_AddTexture(tex.getSampler(), tex.getImageView(), tex.getImageLayout());
    ImGui::Image(x, ImVec2(framebuffer.getWidth(), framebuffer.getHeight()));
    ImGui::End();
    ImGui::PopStyleVar();

    return state.previousSize;
}