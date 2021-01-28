#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <optional>

#include "VulkanInstance.h"

// TODO: Refactor

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

/* Wrapper for the logical device and handles the creation of it for a physical device.
	Contains physical device selection.
	*/
class VulkanDevice {
private:
    // Required extensions
    const std::vector<const char *> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
public:
    VulkanDevice() {}

    ~VulkanDevice() {}

    void init(VulkanInstance &p_Instance, VkSurfaceKHR p_Surface);

    void waitIdle();

    void destroy();

    // Getter
    VulkanInstance *getInstance() { return instance; };

    VkDevice getDevice() const { return device; }

    VkQueue getGraphicsQueue() const { return graphicsQueue; }

    VkQueue getPresentQueue() const { return presentQueue; }

    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

    VkPhysicalDeviceProperties getProperties() const { return properties; }

    uint32_t getGraphicsQueueFamily() const { return graphicsQueueFamily; }

    uint32_t getPresentQueueFamily() const { return presentQueueFamily; }

    // Wrapper for external calls
    bool checkDeviceExtensionSupport();

    QueueFamilyIndices findQueueFamilies();

    bool isDeviceSuitable();

    SwapChainSupportDetails querySwapChainSupport();

    VkFormat
    findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

private:
    bool checkDeviceExtensionSupport(VkPhysicalDevice phdevice);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice phdevice);

    bool isDeviceSuitable(VkPhysicalDevice phdevice);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice phdevice);

    VkFormat
    findSupportedFormat(VkPhysicalDevice phdevice, const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                        VkFormatFeatureFlags features);

    void pickPhysicalDevice();

    void createLogicalDevice();

private:

    VulkanInstance *instance = nullptr;

    VkSurfaceKHR surface = {};

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkDevice device = {};

    uint32_t graphicsQueueFamily;
    uint32_t presentQueueFamily;

    VkQueue graphicsQueue = {};
    VkQueue presentQueue = {};

    VkPhysicalDeviceProperties properties = {};
};

