#pragma once
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

struct MousePos {
    int x;
    int y;
};

struct WindowDimensions {
    int width;
    int height;
};

class Window {
public:
    explicit Window(uint32_t width = 1200, uint32_t height = 800);

    ~Window() = default;

    void poolEvents();

    bool shouldClose() { return glfwWindowShouldClose(window); }

    GLFWwindow *getWindow() { return window; }

    void close() { glfwSetWindowShouldClose(window, GLFW_TRUE); }

    void cleanup();

    bool isKeyDown(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }

    bool isKeyUp(int key) { return glfwGetKey(window, key) == GLFW_RELEASE; }

    MousePos getDeltaMouse() const {
        return MousePos{mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y};
    }

    MousePos getMousePos() { return Window::mousePos; }

    bool isMouseButtonDown(int button) { return glfwGetMouseButton(window, button) == GLFW_PRESS; }

    bool isMouseButtonUp(int button) { return glfwGetMouseButton(window, button) == GLFW_RELEASE; }

    // Rendering specific code

    void setFrameBufferResized(bool b);

    bool getFrameBufferResize() const { return framebufferResized; }

    WindowDimensions  getFrameBufferSize();

    // Vulkan specific code
    void createSurface(const VkInstance &instance, VkSurfaceKHR *surface);

private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

private:
    GLFWwindow *window = nullptr;

    bool framebufferResized = false;

private: // Static members
    MousePos lastMousePos;
    MousePos mousePos;
};

