#pragma once


#include "src/renderer/vulkan/command/VulkanCommandPool.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

class VulkanContext {
private:
    VulkanContext(Window &window, VulkanInstance &&instance, VulkanDevice &&device, VkSurfaceKHR surface,
                  VulkanSwapChain &&swapChain, VulkanCommandPool &&commandPool);

public: // Methods
    ~VulkanContext() = default;

    VulkanContext(const VulkanContext &o) = delete;

    VulkanContext &operator=(const VulkanContext &o) = delete;

    VulkanContext(VulkanContext &&o) noexcept;

    VulkanContext &operator=(VulkanContext &&o) = delete;

    static VulkanContext Create(Window &window);

    inline const VulkanDevice &getDevice() const { return device; }

    inline const VulkanInstance &getInstance() const { return instance; }

    inline const VulkanCommandPool &getCommandPool() const { return commandPool; }

private:
    Window &window;
    VulkanInstance instance;
    VulkanDevice device;
    VkSurfaceKHR surface;
    VulkanSwapChain swapChain;
    VulkanCommandPool commandPool;
};

