#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <vector>

class VulkanInstance {
public:
    VulkanInstance();

    VulkanInstance(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                   std::vector<const char *> validationLayers);

    ~VulkanInstance() = default;

    static VulkanInstance create(std::vector<const char *> validationLayers = {});

    static void destroy(VulkanInstance &instance);

    VkInstance getInstance() const { return instance; }

    const std::vector<const char *> getValidationLayers() const { return validationLayers; }

private:
    static bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);

    static std::vector<const char *> getRequiredExtensions(bool enableValidationLayers);

    static void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT &debugMessenger,
                                    VkDebugUtilsMessengerCreateInfoEXT createInfo);

    static VkResult
    CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

private:
    VkInstance instance = {};
    VkDebugUtilsMessengerEXT debugMessenger = {};
    std::vector<const char *> validationLayers = {};
};

