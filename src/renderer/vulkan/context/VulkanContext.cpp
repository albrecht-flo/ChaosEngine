#include "VulkanContext.h"

#include "src/renderer/window/Window.h"

#include <vulkan/vulkan.h>

#include <stdexcept>

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanContext::VulkanContext(const Window &window)
        : window(window),
          instance(VulkanInstance::Create({"VK_LAYER_KHRONOS_validation"}, "Hello Triangle", "Foo Bar")),
          surface(window.createSurface(instance.vk())),
          device(VulkanDevice::Create(instance, surface)),
          commandPool(VulkanCommandPool::Create(device)),
          swapChain(VulkanSwapChain::Create(window, device, surface)),
          memory(device, commandPool) {}


void VulkanContext::recreateSwapChain() {
    surface = window.createSurface(instance.vk());
    swapChain.recreate(surface);
}