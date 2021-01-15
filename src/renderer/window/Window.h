#pragma once
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

struct MousePos {
    int x;
    int y;
};

class Window {
public:
    Window(uint32_t width = 1200, uint32_t height = 800);

    ~Window();

    void poolEvents();

    bool shouldClose() { return glfwWindowShouldClose(m_window); }

    GLFWwindow *getWindow() { return m_window; }

    bool getFrameBufferResize() { return m_framebufferResized; }

    void close() { glfwSetWindowShouldClose(m_window, GLFW_TRUE); }

    void cleanup();

    void setFrameBufferResized(bool b);

    bool isKeyDown(int key) { return glfwGetKey(m_window, key) == GLFW_PRESS; }

    bool isKeyUp(int key) { return glfwGetKey(m_window, key) == GLFW_RELEASE; }

    MousePos getDeltaMouse() {
        return MousePos{mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y};
    }

    MousePos getMousePos() { return Window::mousePos; }

    bool isMouseButtonDown(int button) { return glfwGetMouseButton(m_window, button) == GLFW_PRESS; }

    bool isMouseButtonUp(int button) { return glfwGetMouseButton(m_window, button) == GLFW_RELEASE; }

private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

private:
    GLFWwindow *m_window = nullptr;

    bool m_framebufferResized = false;

private: // Static members
    MousePos lastMousePos;
    MousePos mousePos;
};

