#include "VulkanContext.h"

#include <stdexcept>

VulkanContext::VulkanContext(Window &window, VulkanInstance &&instance, VulkanDevice &&device, VkSurfaceKHR surface,
                             VulkanSwapChain &&swapChain) :
        window(window),
        instance(std::move(instance)),
        device(std::move(device)),
        surface(surface),
        swapChain(std::move(swapChain)) {}


VulkanContext VulkanContext::Create(Window &window) {
    auto instance = VulkanInstance::Create({"VK_LAYER_KHRONOS_validation"}, "Hello Triangle", "Foo Bar");
    auto surface = window.createSurface(instance.getInstance());

    auto device = VulkanDevice::Create(instance, surface);
    auto swapChain = VulkanSwapChain::Create(window, device, surface);

    return VulkanContext{window, std::move(instance), std::move(device), surface, std::move(swapChain)};
}