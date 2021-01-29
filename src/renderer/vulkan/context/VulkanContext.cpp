#include "VulkanContext.h"

#include "src/renderer/window/Window.h"

#include <vulkan/vulkan.h>

#include <stdexcept>

VulkanContext VulkanContext::Create(const Window &window) {
    auto instance = VulkanInstance::Create({"VK_LAYER_KHRONOS_validation"}, "Hello Triangle", "Foo Bar");
    auto surface = window.createSurface(instance.vk());

    auto device = VulkanDevice::Create(instance, surface);

    auto commandPool = VulkanCommandPool::Create(device);

    return VulkanContext{window, std::move(instance), surface, std::move(device), (std::move(commandPool))};
}


VulkanContext::VulkanContext(const Window &window, VulkanInstance &&instance, VkSurfaceKHR surface,
                             VulkanDevice &&device, VulkanCommandPool &&commandPool) :
        window(window),
        instance(std::move(instance)),
        surface(surface),
        device(std::move(device)),
        commandPool(std::move(commandPool)),
        memory(device, commandPool) {}


VulkanContext::VulkanContext(VulkanContext &&o) noexcept
        : window(o.window), instance(std::move(o.instance)), surface(o.surface), device(std::move(o.device)),
          commandPool(std::move(o.commandPool)), memory(std::move(o.memory)) {}
