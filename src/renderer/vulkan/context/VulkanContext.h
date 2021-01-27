#pragma once


#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

class VulkanContext {
private: // Methods
    explicit VulkanContext(Window &window, VulkanDevice &&device,
                           VulkanInstance &&instance, VulkanSwapChain &&swapChain,
                           VkSurfaceKHR surface);

public: // Methods
    ~VulkanContext() = default;

    VulkanContext(const VulkanContext &o) = delete;

    VulkanContext &operator=(const VulkanContext &o) = delete;

    VulkanContext(VulkanContext &&o) : window(o.window), device(o.device), instance(o.instance),
                                       swapChain(o.swapChain), surface(o.surface) {};

    static VulkanContext Create(Window &window);

    inline VkDevice getVkDevice() const { return device.getDevice(); }

    inline VkInstance getVkInstance() const { return instance.getInstance(); }

    inline VkFormat getSwapChainFormat() const { return swapChain.getFormat(); }

    inline VkExtent2D getSwapChainExtent() const { return swapChain.getExtent(); }

private:
    Window &window;
    VulkanDevice device;
    VulkanInstance instance;
    VulkanSwapChain swapChain;
    VkSurfaceKHR surface;
};

