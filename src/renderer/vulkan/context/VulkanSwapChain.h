#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Window;

class VulkanDevice;

class VulkanImageView;

class VulkanSwapChain {
private:
    VulkanSwapChain(const Window &window, const VulkanDevice &device, VkSurfaceKHR surface,
                    VkSwapchainKHR swapChain, VkFormat swapChainImageFormat, VkExtent2D swapChainExtent,
                    std::vector<VkImage> &&swapChainImages, std::vector<VulkanImageView> &&swapChainImageViews);


public:
    void destroy();

    ~VulkanSwapChain();

    VulkanSwapChain(const VulkanSwapChain &o) = delete;

    VulkanSwapChain &operator=(const VulkanSwapChain &o) = delete;

    VulkanSwapChain(VulkanSwapChain &&o) noexcept;

    VulkanSwapChain &operator=(VulkanSwapChain &&o) = delete;

    void recreate(VkSurfaceKHR mSurface);

    static VulkanSwapChain Create(const Window &mWindow, const VulkanDevice &mDevice, VkSurfaceKHR mSurface);

    [[nodiscard]] inline uint32_t size() const { return static_cast<uint32_t>(swapChainImages.size()); }

    [[nodiscard]] inline VkSwapchainKHR getSwapChain() const { return swapChain; }

    [[nodiscard]] inline VkFormat getFormat() const { return swapChainImageFormat; }

    [[nodiscard]] inline VkExtent2D getExtent() const { return swapChainExtent; }

    [[nodiscard]] inline uint32_t getWidth() const { return swapChainExtent.width; }

    [[nodiscard]] inline uint32_t getHeight() const { return swapChainExtent.height; }

    [[nodiscard]] inline const std::vector<VulkanImageView> &getImageViews() const { return swapChainImageViews; }

private:
    const Window &window;
    const VulkanDevice &device;
    VkSurfaceKHR surface;


    // All objects regarding the swapchain
    VkSwapchainKHR swapChain = {};
    VkFormat swapChainImageFormat = {};
    VkExtent2D swapChainExtent = {};
    std::vector<VkImage> swapChainImages;
    std::vector<VulkanImageView> swapChainImageViews;

};