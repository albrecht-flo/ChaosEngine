#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <vector>

#include "VulkanDevice.h"
#include "src/renderer/window/Window.h"

class VulkanSwapChain {
private:
public:
    VulkanSwapChain(Window &window, VulkanDevice &device, VkSurfaceKHR surface,
                    VkSwapchainKHR swapChain, VkFormat swapChainImageFormat, VkExtent2D swapChainExtent,
                    std::vector<VkImage> &&swapChainImages, std::vector<VkImageView> &&swapChainImageViews);

    void destroy();

public:

    ~VulkanSwapChain();

    VulkanSwapChain(const VulkanSwapChain &o) = delete;

    VulkanSwapChain &operator=(const VulkanSwapChain &o) = delete;

    VulkanSwapChain(VulkanSwapChain &&o);

    VulkanSwapChain &operator=(VulkanSwapChain &&o) = delete;


    static VulkanSwapChain Create(Window &mWindow, VulkanDevice &mDevice, VkSurfaceKHR &mSurface);


    void reinit();

    uint32_t size() const { return static_cast<uint32_t>(swapChainImages.size()); }

    VkSwapchainKHR vSwapChain() const { return swapChain; }

    VkFormat getFormat() const { return swapChainImageFormat; }

    VkExtent2D getExtent() const { return swapChainExtent; }

    std::vector<VkImageView> &getImageViews() { return swapChainImageViews; }

private:
    Window &window;
    VulkanDevice &device;
    VkSurfaceKHR surface;


    // All objects regarding the swapchain
    VkSwapchainKHR swapChain = {};
    VkFormat swapChainImageFormat = {};
    VkExtent2D swapChainExtent = {};
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

};