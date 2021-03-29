#pragma once

#include "Engine/src/renderer/api/Framebuffer.h"

#include <functional>
#include <imgui.h>

class Window;

class CustomImGui {
private:
    struct CustomImGuiState {
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

    static ImVec2 RenderSceneViewport(const Renderer::Framebuffer &framebuffer, const std::string &title = "Viewport");

private:
    static CustomImGuiState state;
};
