#include "VulkanDevice.h"

#include <vector>
#include <set>
#include <stdexcept>

/* Checks if a GPU supports all required extensions. */
bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice phdevice) {
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
QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice phdevice) {
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
        vkGetPhysicalDeviceSurfaceSupportKHR(phdevice, i, m_surface, &presentSupport);

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

/* Get the required swapchain informations. (in SwapChainSupportDetails) */
SwapChainSupportDetails VulkanDevice::querySwapChainSupport(VkPhysicalDevice phdevice) {
    SwapChainSupportDetails details;

    // Get all informations
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phdevice, m_surface, &details.capabilities);

    // Get format
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(phdevice, m_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phdevice, m_surface, &formatCount, details.formats.data());
    }

    // Get present mode
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(phdevice, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(phdevice, m_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}


VkFormat VulkanDevice::findSupportedFormat(VkPhysicalDevice phdevice,
                                           const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                           VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("VULKAN: Failed to find supported format!");
}

/* Checks if a GPU is suitable for this application. */
bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice phdevice) {
    // Get all available queue families (Graphics, Compute, Copy, etc.)
    QueueFamilyIndices indices = findQueueFamilies(phdevice);

    // Check if the extensions are supported
    bool extensionsSupported = checkDeviceExtensionSupport(phdevice);

    // Check if there is a suitable swap chain available
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(phdevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // Get available device features
    VkPhysicalDeviceFeatures supportedDeviceFeatures;
    vkGetPhysicalDeviceFeatures(phdevice, &supportedDeviceFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate
           && supportedDeviceFeatures.samplerAnisotropy;
}

/* Selectss a physical device (GPU) which supports the required features. */
void VulkanDevice::pickPhysicalDevice() {
    // Get all available GPUs
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance->getInstance(), &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance->getInstance(), &deviceCount, devices.data());

    for (const auto &device : devices) {
        // Take the first one that fits
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    // Store the supported features
    vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);

}

/* Creates the logical vulkan device from the physical device. */
void VulkanDevice::createLogicalDevice() {
    // Get queues to be created
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

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
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    // If required specify validation layers
    if (!m_instance->getValidationLayers().empty()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_instance->getValidationLayers().size());
        createInfo.ppEnabledLayerNames = m_instance->getValidationLayers().data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    // Retrieve queues
    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);

    m_graphicsQueueFamily = indices.graphicsFamily.value();
    m_presentQueueFamily = indices.presentFamily.value();
}


/* Waits for all processing done on this device to finish. */
void VulkanDevice::waitIdle() {
    vkDeviceWaitIdle(m_device);
}

// wraper for external calls

bool VulkanDevice::checkDeviceExtensionSupport() {
    return checkDeviceExtensionSupport(m_physicalDevice);
}

QueueFamilyIndices VulkanDevice::findQueueFamilies() {
    return findQueueFamilies(m_physicalDevice);
}

bool VulkanDevice::isDeviceSuitable() {
    return isDeviceSuitable(m_physicalDevice);
}

SwapChainSupportDetails VulkanDevice::querySwapChainSupport() {
    return querySwapChainSupport(m_physicalDevice);
}

VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                           VkFormatFeatureFlags features) {
    return findSupportedFormat(m_physicalDevice, candidates, tiling, features);
}

void VulkanDevice::init(VulkanInstance &instance_p, VkSurfaceKHR surface_p) {
    m_instance = &instance_p;
    m_surface = surface_p;
    // Select a graphics card that supports our features
    pickPhysicalDevice();
    // Create the logical device for this GPU
    createLogicalDevice();
}

void VulkanDevice::destroy() {
    vkDestroyDevice(m_device, nullptr);
}