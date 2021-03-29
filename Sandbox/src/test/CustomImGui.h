#pragma once

#include <functional>

class Window;
namespace CustomImGui {
    void ImGuiEnableDocking(const std::function<void(void)>& menuCallback);

    void RenderLogWindow();
}
