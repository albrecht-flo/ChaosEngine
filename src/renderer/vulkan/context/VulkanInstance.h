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

    void init(std::vector<const char *> validationLayers = {});

    static void destroy(VulkanInstance &instance);

    VkInstance getInstance() const { return instance; }

    const std::vector<const char *> getValidationLayers() const { return validationLayers; }

private:
    VkInstance instance = {};
    VkDebugUtilsMessengerEXT debugMessenger = {};
    std::vector<const char *> validationLayers = {};
};

