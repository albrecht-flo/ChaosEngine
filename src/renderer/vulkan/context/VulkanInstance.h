#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>

class VulkanInstance {
private:
    VulkanInstance(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                   std::vector<const char *> &&validationLayers);

    void destroy();

public:
    ~VulkanInstance();

    VulkanInstance(const VulkanInstance &o) = delete;

    VulkanInstance &operator=(const VulkanInstance &o) = delete;

    VulkanInstance(VulkanInstance &&o) noexcept;

    VulkanInstance &operator=(VulkanInstance &&o) = delete;


    static VulkanInstance Create(std::vector<const char *> validationLayers, const std::string &applicationName,
                                 const std::string &engineName);


    [[nodiscard]] inline VkInstance vk() const { return instance; }

    [[nodiscard]] inline const std::vector<const char *> &getValidationLayers() const { return validationLayers; }

private:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    std::vector<const char *> validationLayers;
};

