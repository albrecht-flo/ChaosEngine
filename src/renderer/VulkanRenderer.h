#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "window/Window.h"

#include "general/VulkanInstance.h"
#include "general/VulkanDevice.h"
#include "general/VulkanSwapChain.h"

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