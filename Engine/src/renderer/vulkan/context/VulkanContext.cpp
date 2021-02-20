#include "VulkanContext.h"

#include "Engine/src/renderer/window/Window.h"

#include <vulkan/vulkan.h>

#include <stdexcept>


static std::vector<VulkanCommandBuffer>
createPrimaryCommandBuffers(const VulkanDevice &device, const VulkanCommandPool &commandPool, uint32_t swapChainSize) {
    std::vector<VulkanCommandBuffer> primaryCommandBuffers;
    primaryCommandBuffers.reserve(swapChainSize);
    for (uint32_t i = 0; i < swapChainSize; ++i) {
        primaryCommandBuffers.emplace_back(
                VulkanCommandBuffer::Create(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY));
    }

    return std::move(primaryCommandBuffers);
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanContext::VulkanContext(Window &window)
        : window(window),
          instance(VulkanInstance::Create({"VK_LAYER_KHRONOS_validation"}, "Hello Triangle", "Foo Bar")),
          surface(window.createSurface(instance.vk())),
          device(VulkanDevice::Create(instance, surface)),
          commandPool(VulkanCommandPool::Create(device)),
          swapChain(VulkanSwapChain::Create(window, device, surface)),
          memory(device, commandPool),
          primaryCommandBuffers(createPrimaryCommandBuffers(device, commandPool, maxFramesInFlight)),
          frame(VulkanFrame::Create(window, *this, maxFramesInFlight)) {}


void VulkanContext::recreateSwapChain() {
    surface = window.createSurface(instance.vk());
    swapChain.recreate(surface);
}

bool VulkanContext::flushCommands() {
    bool swapChainOk = frame.render(currentFrame, primaryCommandBuffers[currentFrame]);
    if (!swapChainOk) {
        device.waitIdle();
        recreateSwapChain();
    }
    currentFrame = (currentFrame < maxFramesInFlight - 1) ? currentFrame + 1 : 0;
    currentSwapChainImage = (currentSwapChainImage < swapChain.size() - 1) ? currentSwapChainImage + 1
                                                                           : 0;

    return swapChainOk;
}
