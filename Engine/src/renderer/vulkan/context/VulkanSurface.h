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


    [[nodiscard]] inline VkSurfaceKHR vk() const { return surface; }

private:
    void destroy() {
        if (surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance.vk(), surface, nullptr);
            surface = VK_NULL_HANDLE;
        }
    }

private:
    const VulkanInstance &instance;
    VkSurfaceKHR surface;
};

