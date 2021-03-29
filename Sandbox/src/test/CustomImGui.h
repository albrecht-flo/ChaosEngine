#pragma once

#include "Engine/src/renderer/api/Framebuffer.h"
#include <functional>
#include <imgui.h>

class Window;

class CustomImGui {
private:
    struct CustomImGuiState {
        bool followLog = true;
        ImVec2 previousSize = ImVec2(0.0f, 0.0f);
    };

public:
    static void ImGuiEnableDocking(const std::function<void(void)> &menuCallback);

    static void RenderLogWindow(const std::string &title = "Log");

    static ImVec2 renderViewport(const Renderer::Framebuffer &framebuffer);

private:
    static CustomImGuiState state;
};
