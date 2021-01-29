#include "Window.h"

#include <stdexcept>

void Window::framebufferResizeCallback(GLFWwindow *window, int /*width*/, int /*height*/) {
    auto w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    w->setFrameBufferResized(true);
}

Window::Window(uint32_t width, uint32_t height) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, "RenderEngine", nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);

    // Setup callbacks
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Window::poolEvents() {
    glfwPollEvents();

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    Window::lastMousePos = Window::mousePos;
    Window::mousePos.x = static_cast<int>(x);
    Window::mousePos.y = static_cast<int>(y);
}

void Window::cleanup() {
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
        throw std::runtime_error("VULKAN: failed to create window surface!");
    }

    return surface;
}
