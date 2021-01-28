#pragma once


#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

class VulkanContext {
private:
    VulkanContext(Window &window, VulkanInstance &&instance, VulkanDevice &&device, VkSurfaceKHR surface,
                  VulkanSwapChain &&swapChain);

public: // Methods
    ~VulkanContext() = default;

    VulkanContext(const VulkanContext &o) = delete;

    VulkanContext &operator=(const VulkanContext &o) = delete;

    VulkanContext(VulkanContext &&o) noexcept: window(o.window), device(std::move(o.device)),
                                               instance(std::move(o.instance)),
                                               swapChain(std::move(o.swapChain)), surface(o.surface) {};

    VulkanContext &operator=(VulkanContext &&o) = delete;

    static VulkanContext Create(Window &window);

    inline VkDevice getVkDevice() const { return device.getDevice(); }

    inline VkInstance getVkInstance() const { return instance.getInstance(); }

    inline VkFormat getSwapChainFormat() const { return swapChain.getFormat(); }

    inline VkExtent2D getSwapChainExtent() const { return swapChain.getExtent(); }

private:
    Window &window;
    VulkanInstance instance;
    VulkanDevice device;
    VkSurfaceKHR surface;
    VulkanSwapChain swapChain;
};

