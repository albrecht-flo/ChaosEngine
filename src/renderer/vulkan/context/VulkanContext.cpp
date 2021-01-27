#include "VulkanContext.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

VulkanContext::VulkanContext(Window &window, VulkanDevice &&device, VulkanInstance &&instance,
                             VulkanSwapChain &&swapChain, VkSurfaceKHR surface) :
        window(window),
        device(device),
        instance(instance),
        swapChain(swapChain),
        surface(surface) {}


VulkanContext VulkanContext::Create(Window &window) {
    VulkanInstance instance = VulkanInstance::create({"VK_LAYER_KHRONOS_validation"});
    VkSurfaceKHR surface{};
    window.createSurface(instance.getInstance(), &surface);
    VulkanDevice device = VulkanDevice();
    device.init(instance, surface);
    VulkanSwapChain swapChain{device, surface, window};
    swapChain.init();

    return VulkanContext(window, std::move(device), std::move(instance), std::move(swapChain), surface);
}