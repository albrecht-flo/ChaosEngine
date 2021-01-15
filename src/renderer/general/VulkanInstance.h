#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <vector>

class VulkanInstance {
public:
    VulkanInstance();

    VulkanInstance(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                   std::vector<const char *> validationLayers);

    ~VulkanInstance();

    static VulkanInstance create(std::vector<const char *> validationLayers = {});

    static void destroy(VulkanInstance &instance);

    VkInstance getInstance() const { return m_instance; }

    const std::vector<const char *> getValidationLayers() const { return m_validationLayers; }

private:
    static bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);

    static std::vector<const char *> getRequiredExtensions(bool enableValidationLayers);

    static void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT &debugMessenger,
                                    VkDebugUtilsMessengerCreateInfoEXT createInfo);

    static VkResult
    CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

private:
    VkInstance m_instance = {};
    VkDebugUtilsMessengerEXT m_debugMessenger = {};
    std::vector<const char *> m_validationLayers = {};
};

