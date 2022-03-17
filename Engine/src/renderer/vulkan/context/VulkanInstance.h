#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <optional>

class VulkanInstance {
private:
    VulkanInstance(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                   std::vector<const char *> validationLayers);

    void destroy();

public:
    ~VulkanInstance();

    VulkanInstance(const VulkanInstance &o) = delete;

    VulkanInstance &operator=(const VulkanInstance &o) = delete;

    VulkanInstance(VulkanInstance &&o) noexcept;

    VulkanInstance &operator=(VulkanInstance &&o) = delete;


    static VulkanInstance
    Create(const std::vector<const char *> &validationLayers, const std::string &applicationName,
           const std::string &engineName);


    [[nodiscard]] inline VkInstance vk() const { return instance; }

    [[nodiscard]] inline const std::vector<const char *> &getValidationLayers() const { return validationLayers; }


    // Debug calls
#ifndef NDEBUG
public:
    void
    setDebugName(VkDevice device, VkObjectType type, uint64_t handle, const std::optional<std::string> &name) const;

private:
    inline void setDebugUtilsObjectNameEXT(VkDevice device, VkDebugUtilsObjectNameInfoEXT *objectNameInfo) const {
        auto pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)
                vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
        pfnSetDebugUtilsObjectNameEXT(device, objectNameInfo);
    }

#else
    public:
        void
        setDebugName(VkDevice, VkObjectType, uint64_t, const std::optional<std::string>&) const
        {}

#endif

private:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    std::vector<const char *> validationLayers;
};

