#pragma once


#include "src/renderer/vulkan/command/VulkanCommandPool.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

/// This class holds all vulkan context that is constant for the whole execution of the application.
class VulkanContext {
private:
    VulkanContext(const Window &window, VulkanInstance &&instance, VulkanDevice &&device, VkSurfaceKHR surface,
                  VulkanCommandPool &&commandPool);

public: // Methods
    ~VulkanContext() = default;

    VulkanContext(const VulkanContext &o) = delete;

    VulkanContext &operator=(const VulkanContext &o) = delete;

    VulkanContext(VulkanContext &&o) noexcept;

    VulkanContext &operator=(VulkanContext &&o) = delete;

    static VulkanContext Create(const Window &window);

    inline const VulkanDevice &getDevice() const { return device; }

    inline const VulkanInstance &getInstance() const { return instance; }

    inline VkSurfaceKHR getSurface() const { return surface; }

    inline const VulkanCommandPool &getCommandPool() const { return commandPool; }

private:
    const Window &window;
    VulkanInstance instance;
    VulkanDevice device;
    VkSurfaceKHR surface;
    VulkanCommandPool commandPool;
};

