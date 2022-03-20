#include "VulkanContext.h"

#include "Engine/src/core/utils/Logger.h"

#include <stdexcept>
#include <cstdio>

static std::vector<VulkanCommandBuffer>
createPrimaryCommandBuffers(const VulkanInstance &instance, const VulkanDevice &device,
                            const VulkanCommandPool &commandPool, uint32_t swapChainSize) {
    std::vector<VulkanCommandBuffer> primaryCommandBuffers;
    primaryCommandBuffers.reserve(swapChainSize);
    for (uint32_t i = 0; i < swapChainSize; ++i) {
        primaryCommandBuffers.emplace_back(
                VulkanCommandBuffer::Create(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY));
        char name[sizeof("PrimaryCommandBuffer-0")];
        snprintf(name, sizeof(name), "PrimaryCommandBuffer-%u", i & 0x7);
        instance.setDebugName(device.vk(), VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t) primaryCommandBuffers.back().vk(),
                              name);
    }

    return primaryCommandBuffers;
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanContext::VulkanContext(Window &window)
        : window(window),
          instance(VulkanInstance::Create(
                  {"VK_LAYER_KHRONOS_validation"},
                  "Hello Triangle", "Foo Bar")),
          surface(instance, window.createSurface(instance.vk())),
          device(VulkanDevice::Create(instance, surface.vk())),
          swapChain(VulkanSwapChain::Create(window, device, surface.vk())),
          graphicsCommandPool(VulkanCommandPool::Create(device, device.getGraphicsQueueFamilyIndex(),
                                                        device.getGraphicsQueue())),
          primaryCommandBuffers(createPrimaryCommandBuffers(instance, device, graphicsCommandPool, maxFramesInFlight)),
          transferCommandPool(VulkanCommandPool::Create(device, device.getTransferQueueFamilyIndex(),
                                                        device.getTransferQueue())),
          memory(VulkanMemory::Create(device, instance, transferCommandPool)),
          frame(VulkanFrame::Create(window, *this, maxFramesInFlight)) {
    Logger::I("VulkanContext", "Created Vulkan Context");
}

VulkanContext::~VulkanContext() {
    // Clear buffered resources
    for (auto &res: bufferedResourceDestroyQueue) {
        res.resource->destroy();
    }
}

void VulkanContext::beginFrame() const {
    frame.waitUntilCurrentFrameIsFree(currentFrame);
}

void VulkanContext::recreateSwapChain() {
    swapChain.destroy();
    surface = VulkanSurface(instance, window.createSurface(instance.vk()));
    swapChain.recreate(surface.vk());
}

bool VulkanContext::flushCommands() {
    bool swapChainOk = frame.render(currentFrame, primaryCommandBuffers[currentFrame]);
    if (!swapChainOk) {
        device.waitIdle();
        recreateSwapChain();
        currentSwapChainImage = 0;
    } else {
        currentSwapChainImage = (currentSwapChainImage < swapChain.size() - 1) ?
                                currentSwapChainImage + 1 : 0;
    }
    currentFrame = (currentFrame < maxFramesInFlight - 1) ? currentFrame + 1 : 0;
    return swapChainOk;
}

void VulkanContext::destroyBuffered(std::unique_ptr<BufferedGPUResource> resource) {
//    LOG_DEBUG("Resource({0}) scheduled for destruction on frame {1}", resource->toString(), currentFrameCounter);
    bufferedResourceDestroyQueue.emplace_back(std::move(resource), currentFrameCounter);
}

void VulkanContext::tickFrame() {
    // destroy buffered resources
    uint32_t i = 0;
    // If the currentFrame has overflowed to 0... the minus still works because all ints are uint32_t
    while (!bufferedResourceDestroyQueue.empty() &&
           bufferedResourceDestroyQueue.front().frameDeleted == (currentFrameCounter - swapChain.size())) {
        bufferedResourceDestroyQueue.front().resource->destroy();
        bufferedResourceDestroyQueue.pop_front();
//        LOG_DEBUG("Resource destroyed on frame {0}", currentFrameCounter);
        ++i;
    }

    if (i != 0) {
        LOG_DEBUG("[VulkanContext] Cleared {0} buffered resources", i);
    }

    ++currentFrameCounter;
}

