#include "Window.h"

#include <stdexcept>
#include <utility>
#include "Engine/src/core/utils/Logger.h"

static void glfwErrorCallback(int error_code, const char *description) {
    LOG_ERROR("[GLFW] Error: [{0}] :{1}", error_code, description);
}

static void framebufferResizeCallback(GLFWwindow *window, int /*width*/, int /*height*/) {
    auto w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    w->setFrameBufferResized(true);
}

static void scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
    auto w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    auto delta = glm::ivec2{static_cast<int>(xOffset), static_cast<int>(yOffset)};
    w->setScrollDelta(delta);
}

static void keyCallback(GLFWwindow * /*window*/, int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/) {
//    auto w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
}

// ------------------------------------ Class members ------------------------------------------------------------------

Window Window::Create(const std::string &applicationName, int width, int height) {
    Logger::Init(LogLevel::Debug);
    Logger::I("Window", "Logger initialized");

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
    glfwSetScrollCallback(windowPtr, scrollCallback);
    glfwSetKeyCallback(windowPtr, keyCallback);
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
    int winX, winY;
    glfwGetCursorPos(window, &x, &y);
    glfwGetWindowPos(window, &winX, &winY);
    Window::lastMousePos = Window::mousePos;
    Window::mousePos = {static_cast<int>(x), static_cast<int>(y)};
    Window::windowPos = {winX, winY};
}

void Window::destroy() {
    LOG_DEBUG("[Window] Window Destroy");
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::setFrameBufferResized(bool b) {
    framebufferResized = b;
}

glm::ivec2 Window::getFrameBufferSize() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return glm::ivec2{width, height};
}


/* Creates the Vulkan surface from the window. */
VkSurfaceKHR Window::createSurface(const VkInstance &instance) const {
    VkSurfaceKHR surface{};
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create window surface!");
    }

    return surface;
}

std::pair<glm::ivec2, glm::ivec2> Window::getGameWindowExtent() const {
    if (viewportMax.x == 0 && viewportMax.y == 0) {
        auto size = getFrameBufferSize();
        glm::ivec2 windowPos{};
        glfwGetWindowPos(window, &windowPos.x, &windowPos.y);
        return {windowPos, windowPos + size};
    }
    return {viewportMin, viewportMax};
}
