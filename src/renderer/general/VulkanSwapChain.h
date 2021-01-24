#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <vector>

#include "VulkanDevice.h"
#include "../window/Window.h"

class VulkanSwapChain {
public:
    VulkanSwapChain(VulkanDevice &mDevice, VkSurfaceKHR &mSurface, Window &mWindow);

    ~VulkanSwapChain() = default;

    void init();

    void destroy();

    void reinit();

    uint32_t size() const { return static_cast<uint32_t>(swapChainImages.size()); }

    VkSwapchainKHR vSwapChain() const { return swapChain; }

    VkFormat getFormat() const { return swapChainImageFormat; }

    VkExtent2D getExtent() const { return swapChainExtent; }

    std::vector<VkImageView> &getImageViews() { return swapChainImageViews; }

private:
    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats);

    static VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    void createSwapChain();

    void createImageViews();

private:
    VulkanDevice &device;
    Window &window;
    VkSurfaceKHR &surface;

    // All objects regarding the swapchain
    VkSwapchainKHR swapChain = {};
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat = {};
    VkExtent2D swapChainExtent = {};
    std::vector<VkImageView> swapChainImageViews;
};