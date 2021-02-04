#include "VulkanInstance.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <utility>

/* Checks if the driver supports validation layers. */
static bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers) {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers) {
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

/* Gets the necessary extension from glfw. */
static std::vector<const char *> getRequiredExtensions(bool enableValidationLayers) {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

/* Debug callback to print debug messages emitted by vulkan. */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
                                                    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    void * /*pUserData*/) {
//    if(!std::strcmp(pCallbackData->pMessage, "Device Extension:"))
    if(std::string(pCallbackData->pMessage).find("Device Extension:") == std::string::npos)
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

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
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Add all necessary extensions
    auto extensions = getRequiredExtensions(!validationLayers.empty());
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkInstance newInstance{};
    if (vkCreateInstance(&createInfo, nullptr, &newInstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create newInstance!");
    }

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


    VkDebugUtilsMessengerEXT newDebugMessenger = VK_NULL_HANDLE;
    if (!validationLayers.empty())
        setupDebugMessenger(newInstance, newDebugMessenger, debugCreateInfo);

    return VulkanInstance(newInstance, newDebugMessenger, validationLayers);
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

