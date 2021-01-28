#pragma once


#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

class VulkanContext {
public: // Methods
    explicit VulkanContext(Window &window);

    ~VulkanContext() = default;

    VulkanContext(const VulkanContext &o) = delete;

    VulkanContext &operator=(const VulkanContext &o) = delete;

    VulkanContext(VulkanContext &&o) noexcept: window(o.window), device(o.device), instance(o.instance),
                                               swapChain(o.swapChain), surface(o.surface) {};

    VulkanContext &operator=(VulkanContext &&o) = delete;

    void init();

    inline VkDevice getVkDevice() const { return device.getDevice(); }

    inline VkInstance getVkInstance() const { return instance.getInstance(); }

    inline VkFormat getSwapChainFormat() const { return swapChain.getFormat(); }

    inline VkExtent2D getSwapChainExtent() const { return swapChain.getExtent(); }

private:
    Window &window;
    VulkanDevice device;
    VulkanInstance instance;
    VkSurfaceKHR surface;
    VulkanSwapChain swapChain;
};

