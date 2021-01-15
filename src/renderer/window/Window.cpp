#include "Window.h"

void Window::framebufferResizeCallback(GLFWwindow *window, int /*width*/, int /*height*/) {
    auto w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    w->setFrameBufferResized(true);
}

Window::Window(uint32_t width, uint32_t height) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(width, height, "RenderEngine", nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, this);

    // Setup callbacks
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

Window::~Window() {}

void Window::poolEvents() {
    glfwPollEvents();

    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    Window::lastMousePos = Window::mousePos;
    Window::mousePos.x = static_cast<int>(x);
    Window::mousePos.y = static_cast<int>(y);
}


void Window::setFrameBufferResized(bool b) {
    m_framebufferResized = b;
}

void Window::cleanup() {
    glfwDestroyWindow(m_window);

    glfwTerminate();
}
