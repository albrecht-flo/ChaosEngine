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

    return primaryCommandBuffers;
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanContext::VulkanContext(Window &window)
        : window(window),
          instance(VulkanInstance::Create(
                  {"VK_LAYER_KHRONOS_validation"},
                  "Hello Triangle", "Foo Bar")),
          surface(window.createSurface(instance.vk())),
          device(VulkanDevice::Create(instance, surface)),
          commandPool(VulkanCommandPool::Create(device)),
          swapChain(VulkanSwapChain::Create(window, device, surface)),
          memory(device, commandPool),
          primaryCommandBuffers(createPrimaryCommandBuffers(device, commandPool, maxFramesInFlight)),
          frame(VulkanFrame::Create(window, *this, maxFramesInFlight)) {}

VulkanContext::~VulkanContext() {
    // Clear buffered resources
    for (auto &res : bufferedResourceDestroyQueue) {
        res.resource->destroy();
    }
//    bufferedResourceDestroyQueue.clear();

    vkDestroySurfaceKHR(instance.vk(), surface, nullptr);
}

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

void VulkanContext::destroyBuffered(std::unique_ptr<BufferedGPUResource> resource) {
    bufferedResourceDestroyQueue.emplace_back(std::move(resource), currentFrameCounter);
}

void VulkanContext::tickFrame() {
    // destroy buffered resources
    uint32_t i = 0;
    // If the currentFrame has overflowed to 0... the minus still works because all ints are uint32_t
    while (!bufferedResourceDestroyQueue.empty() &&
           bufferedResourceDestroyQueue.front().frameDeleted == currentFrameCounter - maxFramesInFlight) {
        bufferedResourceDestroyQueue.front().resource->destroy();
        bufferedResourceDestroyQueue.pop_front();
        ++i;
    }

    if (i != 0) {
        std::cout << "Cleared " << i << " buffered resources" << std::endl;
    }

    ++currentFrameCounter;
}
