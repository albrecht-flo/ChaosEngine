#include "VulkanDevice.h"

#include "Engine/src/core/utils/Logger.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"

#include <set>
#include <stdexcept>
#include <tuple>
#include <bitset>

/* Checks if a GPU supports all required extensions. */
static bool checkDeviceExtensionSupport(VkPhysicalDevice phdevice, const std::vector<const char *> &deviceExtensions) {
    // Get all available extensions
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(phdevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(phdevice, nullptr, &extensionCount, availableExtensions.data());

    // 'requiredDeviceExtensions' is defined in the header and contains all required extensions
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension: availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    for (const auto &extension : requiredExtensions) {
        LOG_WARN("[Vulkan Device] Missing device extension {}", extension);
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
    for (const auto &queueFamily: queueFamilies) {
        LOG_DEBUG("[Vulkan Device] Queue number: {}", std::to_string(queueFamily.queueCount));
        LOG_DEBUG("[Vulkan Device] Queue flags: {}", std::bitset<32>(queueFamily.queueFlags).to_string());
        // Get graphics queue
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        // Get present queue
        VkBool32 presentSupport = false;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(phdevice, i, surface, &presentSupport) != VK_SUCCESS) {
            throw std::runtime_error("[Vulkan Device] vkGetPhysicalDeviceSurfaceSupportKHR Failed");
        }
        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }

        // Get transfer queue
        if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.transferFamily = i;
        }

        // If we have all we are done
        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    if (!indices.transferFamily) {
        Logger::W("VulkanDevice",
                  "No dedicated Transfer Queue present, reverting back to graphics Queue for transfers.");
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
    for (VkFormat format: candidates) {
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

static bool isTextureFormatSupported(VkPhysicalDevice phDevice, VkFormat format) {
    VkFormatProperties props{};
    vkGetPhysicalDeviceFormatProperties(phDevice, format, &props);
    return props.linearTilingFeatures != 0 && props.optimalTilingFeatures != 0;
}

static bool areRequiredTextureSupported(VkPhysicalDevice phdevice, const std::vector<VkFormat> &formats) {
    for (const auto &format: formats) {
        if (!::isTextureFormatSupported(phdevice, format)) {
            LOG_DEBUG("[Vulkan Device] Missing texture format {}", format);
            return false;
        }
    }
    return true;
}


/* Checks if a GPU is suitable for this application. */
static bool isDeviceSuitable(VkPhysicalDevice phdevice, VkSurfaceKHR surface) {
    LOG_DEBUG("[Vulkan Device] Checking device capabilities of device: {}", (void *) phdevice);
    // Get all available queue families (Graphics, Compute, Copy, etc.)
    QueueFamilyIndices indices = findQueueFamilies(phdevice, surface);
    if (!indices.isComplete()) {
        LOG_DEBUG("[Vulkan Device] Missing queues");
        return false;
    }

    // Check if the extensions are supported
    bool extensionsSupported = checkDeviceExtensionSupport(phdevice, VulkanDevice::requiredDeviceExtensions);
    if (!extensionsSupported) {
        LOG_DEBUG("[Vulkan Device] Missing extensions");
        return false;
    }

    // Check if there is a suitable swap chain available
    bool swapChainAdequate = false;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(phdevice, surface);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    if (!swapChainAdequate) {
        LOG_DEBUG("[Vulkan Device] No suitable swap chain was found");
        return false;
    }

    // Get available device features
    VkPhysicalDeviceFeatures supportedDeviceFeatures;
    vkGetPhysicalDeviceFeatures(phdevice, &supportedDeviceFeatures);
    bool requiredDeviceFeaturesPresent = supportedDeviceFeatures.samplerAnisotropy &&
                                         supportedDeviceFeatures.fillModeNonSolid;
    if (!requiredDeviceFeaturesPresent) {
        LOG_DEBUG("[Vulkan Device] Missing device features");
        return false;
    }

    // Check if our required Texture format are supported

    bool texturesSupported = areRequiredTextureSupported(phdevice, VulkanDevice::requiredTextureFormats);
    if (!texturesSupported) {
        LOG_DEBUG("[Vulkan Device] Missing required textures");
        return false;
    }
    return true;
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

    LOG_INFO("[Vulkan Device] {0} Available GPUS:", deviceCount);
    for (size_t i = 0; i < devices.size(); ++i) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);
        LOG_INFO("[Vulkan Device]     GPU({0}): {1}", i, props.deviceName);
    }

    VkPhysicalDevice physicalDevice{};
    for (size_t i = 0; i < devices.size(); ++i) {
        // Take the first one that fits
        if (isDeviceSuitable(devices[i], surface)) {
            LOG_INFO("[Engine]: -- Using GPU {} --", i);
            physicalDevice = devices[i];
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("[Vulkan] Failed to find a suitable GPU!");
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
    std::set<uint32_t> uniqueQueueFamilies;
    if (indices.transferFamily.has_value())
        uniqueQueueFamilies = {*indices.graphicsFamily, *indices.presentFamily, *indices.transferFamily};
    else
        uniqueQueueFamilies = {*indices.graphicsFamily, *indices.presentFamily};

    float queuePriority = 1.0f; // between 0.0 and 1.0
    for (uint32_t queueFamily: uniqueQueueFamilies) {
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
    deviceFeatures.fillModeNonSolid = VK_TRUE; // Enable Line and Point Primitives
    deviceFeatures.wideLines = VK_TRUE; // Enable Line width > 1.0

    // Create the logical device
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    // Specify queues
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    // Specify device features
    createInfo.pEnabledFeatures = &deviceFeatures;
    // Specify extensions
    createInfo.enabledExtensionCount = static_cast<uint32_t>(VulkanDevice::requiredDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = VulkanDevice::requiredDeviceExtensions.data();
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
    vkGetDeviceQueue(device, *indices.graphicsFamily, 0, &graphicsQueue);
    return std::make_tuple(graphicsQueue, *indices.graphicsFamily);
}

// Retrieve the present queue
static std::tuple<VkQueue, uint32_t> getPresentQueue(VkDevice device, QueueFamilyIndices indices) {
    VkQueue presentQueue{};
    // Retrieve queue
    vkGetDeviceQueue(device, *indices.presentFamily, 0, &presentQueue);
    return std::make_tuple(presentQueue, *indices.presentFamily);
}

// Retrieve the present queue
static std::tuple<VkQueue, uint32_t> getTransferQueue(VkDevice device, QueueFamilyIndices indices) {
    VkQueue transferQueue{};
    // Use a separate graphics queue for transfers
    uint32_t index = (indices.transferFamily == indices.graphicsFamily) ? 1 : 0;
    // Retrieve queue
    vkGetDeviceQueue(device, *indices.transferFamily, index, &transferQueue);
    return std::make_tuple(transferQueue, *indices.transferFamily);
}

// ------------------------------------ Class Methods ------------------------------------------------------------------

const std::vector<const char *> VulkanDevice::requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME, // TODO: This is not supported by RenderDoc !!!
};

const std::vector<VkFormat> VulkanDevice::requiredTextureFormats = {
        VK_FORMAT_R8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_R32_SFLOAT,
        VK_FORMAT_R32G32B32A32_SFLOAT,
};

VulkanDevice VulkanDevice::Create(const VulkanInstance &instance, VkSurfaceKHR surface) {
    // Select a graphics card that supports our features
    auto[physicalDevice, deviceProperties] = ::pickPhysicalDevice(instance, surface);

    // Get queues to be created
    QueueFamilyIndices indices = ::findQueueFamilies(physicalDevice, surface);
    // Create the logical device for this GPU
    auto device = ::createLogicalDevice(physicalDevice, instance, indices);

    auto[graphicsQueue, graphicsQueueFamilyIndex] = ::getGraphicsQueue(device, indices);
    auto[presentQueue, presentQueueFamilyIndex] = ::getPresentQueue(device, indices);
    VkQueue transferQueue{};
    uint32_t transferQueueFamilyIndex = 0;
    if (!indices.transferFamily) {
        transferQueue = graphicsQueue;
        transferQueueFamilyIndex = graphicsQueueFamilyIndex;
    } else {
        auto[tQueue, tQueueFamilyIndex] = ::getTransferQueue(device, indices);
        transferQueue = tQueue;
        transferQueueFamilyIndex = tQueueFamilyIndex;
    }

    assert("Missing required Queue" &&
           (graphicsQueue != VK_NULL_HANDLE && presentQueue != VK_NULL_HANDLE && transferQueue != VK_NULL_HANDLE));
    return VulkanDevice{physicalDevice, device,
                        graphicsQueue, graphicsQueueFamilyIndex,
                        presentQueue, presentQueueFamilyIndex,
                        transferQueue, transferQueueFamilyIndex,
                        indices, deviceProperties, instance};
}

VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphicsQueue,
                           uint32_t graphicsQueueFamily,
                           VkQueue presentQueue, uint32_t presentQueueFamily, VkQueue transferQueue,
                           uint32_t transferQueueFamily,
                           QueueFamilyIndices queueFamilyIndices, VkPhysicalDeviceProperties properties,
                           const VulkanInstance &instance)
        : instance(instance), physicalDevice(physicalDevice), device(device),
          graphicsQueue(graphicsQueue), graphicsQueueFamilyIndex(graphicsQueueFamily),
          presentQueue(presentQueue), presentQueueFamilyIndex(presentQueueFamily),
          transferQueue(transferQueue), transferQueueFamilyIndex(transferQueueFamily),
          queueFamilyIndices(queueFamilyIndices), properties(properties) {}

VulkanDevice::VulkanDevice(VulkanDevice &&o) noexcept
        : instance(o.instance),
          physicalDevice(std::exchange(o.physicalDevice, nullptr)),
          device(std::exchange(o.device, nullptr)),
          graphicsQueue(std::exchange(o.graphicsQueue, nullptr)), graphicsQueueFamilyIndex(o.graphicsQueueFamilyIndex),
          presentQueue(std::exchange(o.presentQueue, nullptr)), presentQueueFamilyIndex(o.presentQueueFamilyIndex),
          transferQueue(std::exchange(o.transferQueue, nullptr)), transferQueueFamilyIndex(o.transferQueueFamilyIndex),
          queueFamilyIndices(std::move(o.queueFamilyIndices)), properties(o.properties) {
}

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
    return ::checkDeviceExtensionSupport(physicalDevice, VulkanDevice::requiredDeviceExtensions);
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(VkSurfaceKHR surface) const {
    return ::findQueueFamilies(physicalDevice, surface);
}

SwapChainSupportDetails VulkanDevice::querySwapChainSupport(VkSurfaceKHR surface) const {
    return ::querySwapChainSupport(physicalDevice, surface);
}

VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                           VkFormatFeatureFlags features) const {
    return ::findSupportedFormat(physicalDevice, candidates, tiling, features);
}

bool VulkanDevice::checkSurfaceAvailability(VkSurfaceKHR newSurface) const {
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, *queueFamilyIndices.graphicsFamily, newSurface,
                                         &presentSupport);
    return presentSupport;
}

bool VulkanDevice::isTextureFormatSupported(VkFormat format) const {
    return ::isTextureFormatSupported(physicalDevice, format);
}