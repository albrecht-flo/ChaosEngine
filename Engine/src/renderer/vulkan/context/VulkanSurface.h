#pragma once

#include <vulkan/vulkan.h>

#include <utility>

class VulkanSurface {
public:
    VulkanSurface(const VulkanInstance &instance, VkSurfaceKHR &&surface)
            : instance(instance), surface(surface) {}

    ~VulkanSurface() { destroy(); }

    VulkanSurface(const VulkanSurface &o) = delete;

    VulkanSurface &operator=(const VulkanSurface &o) = delete;

    VulkanSurface(VulkanSurface &&o) noexcept
            : instance(o.instance), surface(std::exchange(o.surface, nullptr)) {}

    VulkanSurface &operator=(VulkanSurface &&o) noexcept {
        if (&o == this)
            return *this;
        destroy();
        surface = std::exchange(o.surface, nullptr);
        return *this;
    }


    inline VkSurfaceKHR vk() const { return surface; }

private:
    void destroy() {
        if (surface != nullptr)
            vkDestroySurfaceKHR(instance.vk(), surface, nullptr);
    }

private:
    const VulkanInstance &instance;
    VkSurfaceKHR surface;
};

