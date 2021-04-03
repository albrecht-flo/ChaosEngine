#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

/* Stores the required queues. */
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

/* Stores relevant swapchain information*/
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanInstance;

class VulkanSurface;

/**
 * This class is a wrapper for the vulkan logical device and also handles the creation of it for a physical device.
 */
class VulkanDevice {
public:
    static const std::vector<const char *> deviceExtensions;

private:
    VulkanDevice(VkPhysicalDevice physicalDevice,
                 VkDevice device, uint32_t graphicsQueueFamily, uint32_t presentQueueFamily,
                 VkQueue graphicsQueue, VkQueue presentQueue, QueueFamilyIndices queueFamilyIndices,
                 VkPhysicalDeviceProperties properties);

    void destroy();

public:
    ~VulkanDevice();

    VulkanDevice(const VulkanDevice &o) = delete;

    VulkanDevice &operator=(const VulkanDevice &o) = delete;

    VulkanDevice(VulkanDevice &&o) noexcept;

    VulkanDevice &operator=(VulkanDevice &&o) = delete;

    static VulkanDevice Create(const VulkanInstance &instance, VkSurfaceKHR surface);

    void waitIdle() const;

    // Getter
    [[nodiscard]] inline VkDevice vk() const { return device; }

    [[nodiscard]] inline VkQueue getGraphicsQueue() const { return graphicsQueue; }

    [[nodiscard]] inline VkQueue getPresentQueue() const { return presentQueue; }

    [[nodiscard]] inline VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

    [[nodiscard]] inline VkPhysicalDeviceProperties getProperties() const { return properties; }

    [[nodiscard]] inline uint32_t getGraphicsQueueFamily() const { return graphicsQueueFamily; }

    [[nodiscard]] inline uint32_t getPresentQueueFamily() const { return presentQueueFamily; }

    [[nodiscard]] inline QueueFamilyIndices getQueueFamilyIndices() const { return queueFamilyIndices; }

    // Wrapper for external calls
    [[nodiscard]] bool checkDeviceExtensionSupport() const;

    [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface) const;

    [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface) const;

    [[nodiscard]] VkFormat
    findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                        VkFormatFeatureFlags features) const;

    [[nodiscard]] bool checkSurfaceAvailability(VkSurfaceKHR newSurface) const;

private:
    VkPhysicalDevice physicalDevice;

    VkDevice device;

    uint32_t graphicsQueueFamily;
    uint32_t presentQueueFamily;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    QueueFamilyIndices queueFamilyIndices;

    VkPhysicalDeviceProperties properties;
};

