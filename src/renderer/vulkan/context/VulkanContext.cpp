#include "VulkanContext.h"

#include "src/renderer/window/Window.h"

#include <vulkan/vulkan.h>

#include <stdexcept>

static std::vector<VulkanCommandBuffer>
createPrimaryCommandBuffers(const VulkanDevice &device, const VulkanCommandPool &commandPool, uint32_t swapChainSize) {
    std::vector<VulkanCommandBuffer> primaryCommandBuffers;
    primaryCommandBuffers.reserve(swapChainSize);
    for (int i = 0; i < swapChainSize; ++i) {
        primaryCommandBuffers.emplace_back(
                VulkanCommandBuffer::Create(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY));
    }

    return std::move(primaryCommandBuffers);
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanContext VulkanContext::Create(const Window &window) {
    auto instance = VulkanInstance::Create({"VK_LAYER_KHRONOS_validation"}, "Hello Triangle", "Foo Bar");
    auto surface = window.createSurface(instance.vk());

    auto device = VulkanDevice::Create(instance, surface);

    auto commandPool = VulkanCommandPool::Create(device);

    VulkanSwapChain swapChain = VulkanSwapChain::Create(window, device, surface);

    auto primaryCommandBuffers = createPrimaryCommandBuffers(device, commandPool, swapChain.size());

    return VulkanContext{window, std::move(instance), surface, std::move(device), (std::move(commandPool)),
                         std::move(swapChain), std::move(primaryCommandBuffers)};
}


VulkanContext::VulkanContext(const Window &window, VulkanInstance &&instance, VkSurfaceKHR surface,
                             VulkanDevice &&device, VulkanCommandPool &&commandPool, VulkanSwapChain &&swapChain,
                             std::vector<VulkanCommandBuffer> &&primaryCommandBuffers) :
        window(window),
        instance(std::move(instance)),
        surface(surface),
        device(std::move(device)),
        commandPool(std::move(commandPool)),
        swapChain(std::move(swapChain)),
        primaryCommandBuffers(std::move(primaryCommandBuffers)),
        memory(device, commandPool) {}


VulkanContext::VulkanContext(VulkanContext &&o) noexcept
        : window(o.window), instance(std::move(o.instance)), surface(o.surface), device(std::move(o.device)),
          commandPool(std::move(o.commandPool)), swapChain(std::move(o.swapChain)),
          primaryCommandBuffers(std::move(o.primaryCommandBuffers)), memory(std::move(o.memory)) {}
