#pragma once

#include "Engine/src/renderer/api/Framebuffer.h"
#include "Engine/src/core/Scene.h"

#include <imgui.h>

#include <functional>
#include <string>

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

    // Trees
    static bool TreeLeaf(const char *label, uint32_t id, uint32_t *id_ptr);

    static bool TreeNodeBegin(const char *label, uint32_t id, uint32_t *id_ptr);

    static void TreeNodeEnd();

private:
    static CustomImGuiState state;
};
