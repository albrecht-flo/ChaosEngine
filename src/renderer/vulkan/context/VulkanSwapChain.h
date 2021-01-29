#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Window;

class VulkanDevice;

class VulkanSwapChain {
private:
    VulkanSwapChain(const Window &window, const VulkanDevice &device, VkSurfaceKHR surface,
                    VkSwapchainKHR swapChain, VkFormat swapChainImageFormat, VkExtent2D swapChainExtent,
                    std::vector<VkImage> &&swapChainImages, std::vector<VkImageView> &&swapChainImageViews);

public: // TODO: make private
    void destroy();

public:

    ~VulkanSwapChain();

    VulkanSwapChain(const VulkanSwapChain &o) = delete;

    VulkanSwapChain &operator=(const VulkanSwapChain &o) = delete;

    VulkanSwapChain(VulkanSwapChain &&o) noexcept;

    VulkanSwapChain &operator=(VulkanSwapChain &&o) = delete;


    static VulkanSwapChain Create(const Window &mWindow, const VulkanDevice &mDevice, VkSurfaceKHR mSurface);


    void reinit();

    [[nodiscard]] uint32_t size() const { return static_cast<uint32_t>(swapChainImages.size()); }

    [[nodiscard]] VkSwapchainKHR getSwapChain() const { return swapChain; }

    [[nodiscard]] VkFormat getFormat() const { return swapChainImageFormat; }

    [[nodiscard]] VkExtent2D getExtent() const { return swapChainExtent; }
    [[nodiscard]] uint32_t getWidth() const { return swapChainExtent.width; }
    [[nodiscard]] uint32_t getHeight() const { return swapChainExtent.height; }

    std::vector<VkImageView> &getImageViews() { return swapChainImageViews; }

private:
    const Window &window;
    const VulkanDevice &device;
    VkSurfaceKHR surface;


    // All objects regarding the swapchain
    VkSwapchainKHR swapChain = {};
    VkFormat swapChainImageFormat = {};
    VkExtent2D swapChainExtent = {};
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

};