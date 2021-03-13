#include "Window.h"

#include <stdexcept>
#include <iostream>
#include <utility>

static void glfwErrorCallback(int error_code, const char *description) {
    std::cerr << "[GLFW] Error: [" << error_code << "] :" << description << std::endl;
}

static void framebufferResizeCallback(GLFWwindow *window, int /*width*/, int /*height*/) {
    auto w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    w->setFrameBufferResized(true);
}

// ------------------------------------ Class members ------------------------------------------------------------------

Window Window::Create(const std::string &applicationName, uint32_t width, uint32_t height) {
    glfwSetErrorCallback(glfwErrorCallback);

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("[GLFW] Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *windowPtr = glfwCreateWindow(width, height, applicationName.c_str(), nullptr, nullptr);
    if (windowPtr == nullptr) {
        throw std::runtime_error("[GLFW] Failed to create a window!");
    }

    // Setup callbacks
    glfwSetFramebufferSizeCallback(windowPtr, framebufferResizeCallback);

    return Window{windowPtr};
}

Window::Window(GLFWwindow *window)
        : window(window), framebufferResized(false),
          lastMousePos({0, 0}), mousePos({0, 0}) {
    glfwSetWindowUserPointer(window, this);
}

Window::Window(Window &&o) noexcept
        : window(std::exchange(o.window, nullptr)), framebufferResized(o.framebufferResized),
          lastMousePos(o.lastMousePos), mousePos(o.mousePos) {
    glfwSetWindowUserPointer(window, this);
}

void Window::poolEvents() {
    glfwPollEvents();

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    Window::lastMousePos = Window::mousePos;
    Window::mousePos.x = static_cast<int>(x);
    Window::mousePos.y = static_cast<int>(y);
}

void Window::destroy() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::setFrameBufferResized(bool b) {
    framebufferResized = b;
}

WindowDimensions Window::getFrameBufferSize() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return WindowDimensions{width, height};
}


/* Creates the Vulkan surface from the window. */
VkSurfaceKHR Window::createSurface(const VkInstance &instance) const {
    VkSurfaceKHR surface{};
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create window surface!");
    }

    return surface;
}
