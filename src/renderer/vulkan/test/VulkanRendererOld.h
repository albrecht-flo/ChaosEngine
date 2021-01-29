#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "src/renderer/window/Window.h"

#include "src/renderer/vulkan/context/VulkanInstance.h"
#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "src/renderer/vulkan/context/VulkanSwapChain.h"

class VulkanRendererOld {
public:
    VulkanRendererOld(Window &w);

    ~VulkanRendererOld();

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