#include "VulkanContext.h"

#include "src/renderer/window/Window.h"

#include <vulkan/vulkan.h>

#include <stdexcept>

VulkanContext::VulkanContext(Window &window, VulkanInstance &&instance, VulkanDevice &&device, VkSurfaceKHR surface,
                             VulkanSwapChain &&swapChain, VulkanCommandPool &&commandPool) :
        window(window),
        instance(std::move(instance)),
        device(std::move(device)),
        surface(surface),
        swapChain(std::move(swapChain)),
        commandPool(std::move(commandPool)) {}


VulkanContext::VulkanContext(VulkanContext &&o) noexcept
        : window(o.window), device(std::move(o.device)),
          instance(std::move(o.instance)),
          swapChain(std::move(o.swapChain)), surface(o.surface), commandPool(std::move(o.commandPool)) {}

VulkanContext VulkanContext::Create(Window &window) {
    auto instance = VulkanInstance::Create({"VK_LAYER_KHRONOS_validation"}, "Hello Triangle", "Foo Bar");
    auto surface = window.createSurface(instance.getInstance());

    auto device = VulkanDevice::Create(instance, surface);
    auto swapChain = VulkanSwapChain::Create(window, device, surface);

    auto commandPool = VulkanCommandPool::Create(device);

    return VulkanContext{window, std::move(instance), std::move(device), surface, std::move(swapChain),
                         (std::move(commandPool))};
}