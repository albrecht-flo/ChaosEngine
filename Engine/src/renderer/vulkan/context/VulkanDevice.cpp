#include "VulkanDevice.h"

#include "VulkanInstance.h"

#include <iostream>
#include <set>
#include <stdexcept>
#include <tuple>

/* Checks if a GPU supports all required extensions. */
static bool checkDeviceExtensionSupport(VkPhysicalDevice phdevice, const std::vector<const char *> &deviceExtensions) {
    // Get all available extensions
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(phdevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(phdevice, nullptr, &extensionCount, availableExtensions.data());

    // 'deviceExtensions' is defined in the header and contains all required extensions
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

/* Gets a graphics and a present queue out of the available queues. (in QueueFamilyIndices) */
static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice phdevice, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    // Get all available queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phdevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(phdevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
        // Get graphics queue
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        // Get present queue
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(phdevice, i, surface, &presentSupport);

        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }

        // If we have both we are done
        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

/* Get the required swapchain information. (in SwapChainSupportDetails) */
static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice phdevice, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    // Get all information
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phdevice, surface, &details.capabilities);

    // Get format
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(phdevice, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phdevice, surface, &formatCount, details.formats.data());
    }

    // Get present mode
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(phdevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(phdevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}


static VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice,
                                    const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                    VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("[Vulkan] Failed to find supported format!");
}

/* Checks if a GPU is suitable for this application. */
static bool isDeviceSuitable(VkPhysicalDevice phdevice, VkSurfaceKHR surface) {
    // Get all available queue families (Graphics, Compute, Copy, etc.)
    QueueFamilyIndices indices = findQueueFamilies(phdevice, surface);

    // Check if the extensions are supported
    bool extensionsSupported = checkDeviceExtensionSupport(phdevice, VulkanDevice::deviceExtensions);

    // Check if there is a suitable swap chain available
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(phdevice, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // Get available device features
    VkPhysicalDeviceFeatures supportedDeviceFeatures;
    vkGetPhysicalDeviceFeatures(phdevice, &supportedDeviceFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate
           && supportedDeviceFeatures.samplerAnisotropy;
}

/* Selects a physical device (GPU) which supports the required features. */
static std::tuple<VkPhysicalDevice, VkPhysicalDeviceProperties>
pickPhysicalDevice(const VulkanInstance &instance, VkSurfaceKHR surface) {
    // Get all available GPUs
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance.vk(), &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance.vk(), &deviceCount, devices.data());

    std::cout << deviceCount << " Available GPUs:" << std::endl;
    for (const auto &device : devices) {
      VkPhysicalDeviceProperties props;
      vkGetPhysicalDeviceProperties(device, &props);
      std::cout << "    GPU(" << props.deviceID << "): "<< props.deviceName << std::endl;
    }

    VkPhysicalDevice physicalDevice{};
    for (const auto &device : devices) {
        // Take the first one that fits
        if (isDeviceSuitable(device, surface)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    // Store the supported features
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    return std::make_tuple(physicalDevice, properties);

}

/* Creates the logical vulkan device from the physical device. */
static VkDevice
createLogicalDevice(VkPhysicalDevice physicalDevice, const VulkanInstance &instance, QueueFamilyIndices indices) {

    // Create the queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f; // between 0.0 and 1.0
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1; // we only need one queue per family
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Required device features
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE; // needed for texture filtering

    // Create the logical device
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    // Specify queues
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    // Specify device features
    createInfo.pEnabledFeatures = &deviceFeatures;
    // Specify extensions
    createInfo.enabledExtensionCount = static_cast<uint32_t>(VulkanDevice::deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = VulkanDevice::deviceExtensions.data();
    // If required specify validation layers
    if (!instance.getValidationLayers().empty()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(instance.getValidationLayers().size());
        createInfo.ppEnabledLayerNames = instance.getValidationLayers().data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VkDevice device{};
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    return device;
}

// Retrieve the graphics queue
static std::tuple<VkQueue, uint32_t> getGraphicsQueue(VkDevice device, QueueFamilyIndices indices) {
    VkQueue graphicsQueue{};
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    uint32_t graphicsQueueFamily = indices.graphicsFamily.value();
    return std::make_tuple(graphicsQueue, graphicsQueueFamily);
}

// Retrieve the present queue
static std::tuple<VkQueue, uint32_t> getPresentQueue(VkDevice device, QueueFamilyIndices indices) {
    VkQueue presentQueue{};
    // Retrieve queues
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    uint32_t presentQueueFamily = indices.presentFamily.value();
    return std::make_tuple(presentQueue, presentQueueFamily);
}

// ------------------------------------ Class Methods ------------------------------------------------------------------

const std::vector<const char *> VulkanDevice::deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VulkanDevice VulkanDevice::Create(const VulkanInstance &instance, VkSurfaceKHR surface) {
    // Select a graphics card that supports our features
    auto[physicalDevice, deviceProperties] = ::pickPhysicalDevice(instance, surface);

    // Get queues to be created
    QueueFamilyIndices indices = ::findQueueFamilies(physicalDevice, surface);
    // Create the logical device for this GPU
    auto device = ::createLogicalDevice(physicalDevice, instance, indices);

    auto[graphicsQueue, graphicsQueueFamily] = ::getGraphicsQueue(device, indices);
    auto[presentQueue, presentQueueFamily] = ::getPresentQueue(device, indices);

    return VulkanDevice{surface, physicalDevice, device, graphicsQueueFamily, presentQueueFamily,
                        graphicsQueue, presentQueue, deviceProperties};
}

VulkanDevice::VulkanDevice(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice,
                           VkDevice device, uint32_t graphicsQueueFamily, uint32_t presentQueueFamily,
                           VkQueue graphicsQueue, VkQueue presentQueue, VkPhysicalDeviceProperties properties)
        : surface(surface), physicalDevice(physicalDevice), device(device), graphicsQueueFamily(
        graphicsQueueFamily), presentQueueFamily(presentQueueFamily), graphicsQueue(graphicsQueue), presentQueue(
        presentQueue), properties(properties) {}

VulkanDevice::VulkanDevice(VulkanDevice &&o) noexcept
        : surface(std::exchange(o.surface, nullptr)),
          physicalDevice(std::exchange(o.physicalDevice, nullptr)),
          device(std::exchange(o.device, nullptr)),
          graphicsQueueFamily(o.graphicsQueueFamily), presentQueueFamily(o.presentQueueFamily),
          graphicsQueue(std::exchange(o.graphicsQueue, nullptr)),
          presentQueue(std::exchange(o.presentQueue, nullptr)),
          properties(o.properties) {}

VulkanDevice::~VulkanDevice() { destroy(); }

void VulkanDevice::destroy() {
    if (device != nullptr)
        vkDestroyDevice(device, nullptr);
}

/* Waits for all processing done on this device to finish. */
void VulkanDevice::waitIdle() const {
    vkDeviceWaitIdle(device);
}

// wrapper for external calls

bool VulkanDevice::checkDeviceExtensionSupport() const {
    return ::checkDeviceExtensionSupport(physicalDevice, VulkanDevice::deviceExtensions);
}

QueueFamilyIndices VulkanDevice::findQueueFamilies() const {
    return ::findQueueFamilies(physicalDevice, surface);
}

SwapChainSupportDetails VulkanDevice::querySwapChainSupport() const {
    return ::querySwapChainSupport(physicalDevice, surface);
}

VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                           VkFormatFeatureFlags features) const {
    return ::findSupportedFormat(physicalDevice, candidates, tiling, features);
}
