#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <string>

struct MousePos {
    int x;
    int y;
};

struct WindowDimensions {
    int width;
    int height;
};

class Window {
private:
    explicit Window(GLFWwindow *window);

public:
    ~Window() = default;

    Window(const Window &o) = delete;

    Window &operator=(const Window &o) = delete;

    Window(Window &&o) noexcept;

    Window &operator=(Window &&o) = delete;

    static Window
    Create(const std::string &applicationName = "Vulkan Triangle", uint32_t width = 1200, uint32_t height = 800);

    void poolEvents();

    bool shouldClose() { return glfwWindowShouldClose(window); }

    GLFWwindow *getWindow() { return window; }

    void close() { glfwSetWindowShouldClose(window, GLFW_TRUE); }

    void cleanup();

    bool isKeyDown(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }

    bool isKeyUp(int key) { return glfwGetKey(window, key) == GLFW_RELEASE; }

    [[nodiscard]] MousePos getDeltaMouse() const {
        return MousePos{mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y};
    }

    MousePos getMousePos() { return Window::mousePos; }

    bool isMouseButtonDown(int button) { return glfwGetMouseButton(window, button) == GLFW_PRESS; }

    bool isMouseButtonUp(int button) { return glfwGetMouseButton(window, button) == GLFW_RELEASE; }

    // Rendering specific code

    void setFrameBufferResized(bool b);

    [[nodiscard]] bool getFrameBufferResize() const { return framebufferResized; }

    [[nodiscard]] WindowDimensions getFrameBufferSize() const;

    // Vulkan specific code
    [[nodiscard]] VkSurfaceKHR createSurface(const VkInstance &instance) const;

private:
    GLFWwindow *window = nullptr;

    bool framebufferResized = false;
    MousePos lastMousePos;
    MousePos mousePos;
};

