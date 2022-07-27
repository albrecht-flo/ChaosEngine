#include "VulkanInstance.h"

#include "Engine/src/core/utils/Logger.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <cstring>
#include <utility>
#include <set>
#include <optional>

/* Checks if the driver supports validation layers. */
static bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

//    for (const auto &layerProperties : availableLayers) {
//    }

    for (const char *layerName: validationLayers) {
        bool layerFound = false;

        for (const auto &layerProperties: availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    return true;
}

[[maybe_unused]] static bool checkDebugLayerSupport(std::vector<const char *> debugLayers) {
    uint32_t extensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    auto dbgLayer = std::set<std::string>(debugLayers.begin(), debugLayers.end());
    for (const auto &extensionProperties: availableExtensions) {
        dbgLayer.erase(extensionProperties.extensionName);
    }

    return dbgLayer.empty();
}

/* Gets the necessary extension from glfw. */
static std::vector<const char *> getRequiredExtensions(bool enableValidationLayers) {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    LOG_INFO("[VulkanInstance] Required extensions:");
    for (const char *str: extensions) {
        LOG_INFO("[VulkanInstance] \t{}", str);
    }

    return extensions;
}

/* Debug callback to print debug messages emitted by vulkan. */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    void * /*pUserData*/) {
    if (std::string(pCallbackData->pMessage).find("Device Extension:") != std::string::npos)
        return VK_FALSE;

    std::string messageTypePrefix = (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) ? "General" : "";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        messageTypePrefix += "VALI";
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            messageTypePrefix += "|";
    }
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        messageTypePrefix += "PERF";

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        LOG_ERROR("[Vulkan] ({0}): {1}", messageTypePrefix, pCallbackData->pMessage);
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        LOG_WARN("[Vulkan] ({0}): {1}", messageTypePrefix, pCallbackData->pMessage);
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        LOG_INFO("[Vulkan] ({0}): {1}", messageTypePrefix, pCallbackData->pMessage);
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        LOG_TRACE("[Vulkan] ({0}): {1}", messageTypePrefix, pCallbackData->pMessage);

    return VK_FALSE;
}

/* Tries to create a debug messenger while checking if the extension is present. */
static VkResult
CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator,
                             VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/* Creates and attaches the debug messenger. */
static void setupDebugMessenger(VkInstance instance,
                                VkDebugUtilsMessengerEXT &debugMessenger,
                                VkDebugUtilsMessengerCreateInfoEXT createInfo) {

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to set up debug messenger!");
    }
}

// ------------------------------------ Class Methods ------------------------------------------------------------------

/* Creates a vulkan instance with a debug callback and validation layers if requested. */
VulkanInstance
VulkanInstance::Create(const std::vector<const char *> &validationLayers, const std::string &applicationName,
                       const std::string &engineName) {
    // Check the validation layers
    if (!validationLayers.empty() && !checkValidationLayerSupport(validationLayers)) {
        throw std::runtime_error("VULKAN: validation layers requested, but not available!");
    }

    // Create the Instance, takes context information
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = engineName.c_str();
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Add all necessary extensions
    auto extensions = getRequiredExtensions(!validationLayers.empty());
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // If required setup debug callback
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (!validationLayers.empty()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        // Populate debug create Info
        debugCreateInfo = {};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;

        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }


    VkInstance newInstance{};
    if (vkCreateInstance(&createInfo, nullptr, &newInstance) != VK_SUCCESS) {
        throw std::runtime_error("[VulkanInstance] Failed to create newInstance!");
    }

    VkDebugUtilsMessengerEXT newDebugMessenger = VK_NULL_HANDLE;
    if (!validationLayers.empty())
        setupDebugMessenger(newInstance, newDebugMessenger, debugCreateInfo);

    return VulkanInstance{newInstance, newDebugMessenger, validationLayers};
}

VulkanInstance::VulkanInstance(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                               std::vector<const char *> validationLayers) :
        instance(instance), debugMessenger(debugMessenger), validationLayers(std::move(validationLayers)) {}

VulkanInstance::VulkanInstance(VulkanInstance &&o) noexcept:
        instance(std::exchange(o.instance, nullptr)), debugMessenger(std::exchange(o.debugMessenger, nullptr)),
        validationLayers(std::move(o.validationLayers)) {}

VulkanInstance::~VulkanInstance() { destroy(); }


/* Destroys the instance and if present the debug messenger. */
void VulkanInstance::destroy() {
    if (debugMessenger != VK_NULL_HANDLE) {
        // Get destroy function
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) { // if extension is present
            func(instance, debugMessenger, nullptr);
        }
    }
    if (instance != nullptr)
        vkDestroyInstance(instance, nullptr);
}


// ------------------------------------ Debug Members ------------------------------------------------------------------

#ifndef NDEBUG

void VulkanInstance::setDebugName(VkDevice device, VkObjectType type, uint64_t handle,
                                  const std::optional<std::string> &name) const {
    if (!name)
        return;
    VkDebugUtilsObjectNameInfoEXT fenceDgbInfo = {};
    fenceDgbInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    fenceDgbInfo.objectType = type;
    fenceDgbInfo.objectHandle = handle;
    fenceDgbInfo.pObjectName = name->c_str();
    setDebugUtilsObjectNameEXT(device, &fenceDgbInfo);
}

#endif