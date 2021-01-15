#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <vector>

#include "VulkanDevice.h"
#include "../window/Window.h"

class VulkanSwapChain {
public:
    VulkanSwapChain();

    ~VulkanSwapChain();

    void init(VulkanDevice *device, Window *window, VkSurfaceKHR *surface);

    void destroy();

    void reinit();

    uint32_t size() const { return static_cast<uint32_t>(m_swapChainImages.size()); }

    VkSwapchainKHR vSwapChain() const { return m_swapChain; }

    VkFormat getFormat() const { return m_swapChainImageFormat; }

    VkExtent2D getExtent() const { return m_swapChainExtent; }

    std::vector<VkImageView> &getImageViews() { return m_swapChainImageViews; }

private:
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes);

    void createSurface();

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    void createSwapChain();

    void createImageViews();

private:
    VulkanDevice *m_device;
    Window *m_window;
    VkSurfaceKHR *m_surface;

    // All objects regarding the swapchain
    VkSwapchainKHR m_swapChain = {};
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat = {};
    VkExtent2D m_swapChainExtent = {};
    std::vector<VkImageView> m_swapChainImageViews;
};