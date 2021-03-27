#include "CustomImGui.h"

#include "Engine/src/core/Utils/Logger.h"

#include <imgui.h>

#include <vector>
#include <string>

namespace CustomImGui {

    static bool followLog = true;

    void RenderLogWindow() {
        ImGui::Begin("Log");
        if (ImGui::Button("Toggle auto scroll"))
            followLog = !followLog;
        std::vector<std::string> logger = Logger::GetLogBuffer();
        if (ImGui::BeginListBox("##LogList", ImVec2{-FLT_MIN, -FLT_MIN})) {
            for (const auto &str: logger) {
                ImGui::Selectable(str.c_str());
            }

            auto left_up = ImGui::GetWindowPos();
            auto right_down = ImVec2{left_up.x + ImGui::GetWindowWidth(), left_up.y + ImGui::GetWindowHeight()};
            if (ImGui::IsMouseHoveringRect(left_up, right_down) && ImGui::GetIO().MouseWheel != 0)
                followLog = false;
            if (ImGui::IsMouseHoveringRect(left_up, right_down) && ImGui::GetIO().MouseWheel < 0 &&
                ImGui::GetScrollY() == ImGui::GetScrollMaxY())
                followLog = true;

            if (followLog)
                ImGui::SetScrollY(ImGui::GetScrollMaxY());
            ImGui::EndListBox();
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to create log list!");
        }
        ImGui::End();
    }
}