#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "src/renderer/window/Window.h"

#include "context/VulkanInstance.h"
#include "context/VulkanDevice.h"
#include "context/VulkanSwapChain.h"

class VulkanRenderer {
public:
    VulkanRenderer(Window &w);

    ~VulkanRenderer();

    void waitIdle();

    void cleanup();

private:
    void createSurface();

public:
    virtual void drawFrame() = 0;

    virtual void init() = 0;

protected:
    virtual void destroyResources() = 0;

protected:
    Window &window;

    VulkanInstance instance;
    VkSurfaceKHR surface = {};

    VulkanDevice device;
};