#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

class Window {
private:
    explicit Window(GLFWwindow *window);

    void destroy();

public:
    ~Window() { destroy(); };

    Window(const Window &o) = delete;

    Window &operator=(const Window &o) = delete;

    Window(Window &&o) noexcept;

    Window &operator=(Window &&o) = delete;

    static Window
    Create(const std::string &applicationName = "Vulkan Triangle", int width = 1200, int height = 800);

    void poolEvents();

    inline bool shouldClose() { return glfwWindowShouldClose(window); }

    [[nodiscard]] inline GLFWwindow *getWindow() const { return window; }

    inline void close() { glfwSetWindowShouldClose(window, GLFW_TRUE); }

    bool isKeyDown(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }

    bool isKeyUp(int key) { return glfwGetKey(window, key) == GLFW_RELEASE; }

    [[nodiscard]] glm::ivec2 getDeltaMouse() const {
        return glm::ivec2{mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y};
    }

    glm::ivec2 getMousePos() { return mousePos; }

    glm::ivec2 getAbsoluteMousePos() { return windowPos + mousePos; }

    bool isMouseButtonDown(int button) { return glfwGetMouseButton(window, button) == GLFW_PRESS; }

    bool isMouseButtonUp(int button) { return glfwGetMouseButton(window, button) == GLFW_RELEASE; }

    void setScrollDelta(glm::ivec2 delta) { scrollDelta = delta; }

    [[nodiscard]] glm::ivec2 getScrollDelta() const { return scrollDelta; }

    // Rendering specific code
    void setFrameBufferResized(bool b);

    [[nodiscard]] bool getFrameBufferResize() const { return framebufferResized; }

    [[nodiscard]] glm::ivec2 getFrameBufferSize() const;

    void setGameWindowExtent(glm::ivec2 min, glm::ivec2 max) {
        viewportMin = min;
        viewportMax = max;
    }

    [[nodiscard]] std::pair<glm::ivec2, glm::ivec2> getGameWindowExtent() const;

    // Vulkan specific code
    [[nodiscard]] VkSurfaceKHR createSurface(const VkInstance &instance) const;

private:
    GLFWwindow *window = nullptr;

    bool framebufferResized = false;
    glm::ivec2 lastMousePos;
    glm::ivec2 mousePos;
    glm::ivec2 windowPos;
    glm::ivec2 scrollDelta{0, 0};
    glm::ivec2 viewportMin{0, 0};
    glm::ivec2 viewportMax{0, 0};
};

