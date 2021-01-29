#pragma once


#include <src/renderer/vulkan/memory/VulkanMemory.h>
#include "src/renderer/vulkan/command/VulkanCommandPool.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

/// This class holds all vulkan context that is constant for the whole execution of the application.
class VulkanContext {
private:
    VulkanContext(const Window &window, VulkanInstance &&instance, VkSurfaceKHR surface, VulkanDevice &&device,
                  VulkanCommandPool &&commandPool);

public: // Methods
    ~VulkanContext() = default;

    VulkanContext(const VulkanContext &o) = delete;

    VulkanContext &operator=(const VulkanContext &o) = delete;

    VulkanContext(VulkanContext &&o) noexcept;

    VulkanContext &operator=(VulkanContext &&o) = delete;

    static VulkanContext Create(const Window &window);

    [[nodiscard]] inline const VulkanDevice &getDevice() const { return device; }

    [[nodiscard]] inline const VulkanInstance &getInstance() const { return instance; }

    [[nodiscard]] inline VkSurfaceKHR getSurface() const { return surface; }

    [[nodiscard]] inline const VulkanCommandPool &getCommandPool() const { return commandPool; }

    [[nodiscard]] inline const VulkanMemory &getMemory() const { return memory; }

private:
    const Window &window;
    VulkanInstance instance;
    VkSurfaceKHR surface;
    VulkanDevice device;
    VulkanCommandPool commandPool;
    VulkanMemory memory;
};

