#pragma once

#include "Engine/src/renderer/api/GraphicsContext.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "Engine/src/renderer/vulkan/memory/VulkanMemory.h"
#include "Engine/src/renderer/vulkan/command/VulkanCommandPool.h"
#include "Engine/src/renderer/vulkan/command/VulkanCommandBuffer.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanFrame.h"

#include <deque>

/**
 * This class holds all vulkan context that is constant for the whole execution of the application.
 * Because this is referenced throughout the application it **must** not be moved,
 * otherwise the references and pointers to these objects will break.
 *
 * It also wraps the swap buffer functionality with recreation of the swap chain.
 */
class VulkanContext : public Renderer::GraphicsContext {
public:
    explicit VulkanContext(Window &window);

    ~VulkanContext() override;

    VulkanContext(const VulkanContext &o) = delete;

    VulkanContext &operator=(const VulkanContext &o) = delete;

    VulkanContext(VulkanContext &&o) = delete;

    VulkanContext &operator=(VulkanContext &&o) = delete;

    void recreateSwapChain();

    void beginFrame() const override;

    bool flushCommands() override;

    void destroyBuffered(std::unique_ptr<BufferedGPUResource> resource) override;

    void tickFrame() override;

    void waitIdle() override { device.waitIdle(); }

    [[nodiscard]] inline const Window &getWindow() const { return window; }

    [[nodiscard]] inline const VulkanDevice &getDevice() const { return device; }

    [[nodiscard]] inline const VulkanInstance &getInstance() const { return instance; }

    [[nodiscard]] inline VkSurfaceKHR getSurface() const { return surface.vk(); }

    [[nodiscard]] inline const VulkanMemory &getMemory() const { return memory; }

    [[nodiscard]] inline const VulkanSwapChain &getSwapChain() const { return swapChain; }

    [[nodiscard]] inline uint32_t getCurrentFrame() const { return currentFrame; }

    [[nodiscard]] inline uint32_t getCurrentSwapChainFrame() const { return currentSwapChainImage; }

    [[nodiscard]] inline const VulkanCommandBuffer &
    getCurrentPrimaryCommandBuffer() const { return primaryCommandBuffers[currentFrame]; }

    [[nodiscard]] const VulkanCommandPool &getTransferCommandPool() const { return transferCommandPool; }

    [[nodiscard]] const VulkanCommandPool &getGraphicsCommandPool() const { return graphicsCommandPool; }

// ------------------------------------ Debug members ------------------------------------------------------------------

    inline void setDebugName(VkObjectType type, uint64_t handle, const std::optional<std::string> &name) const {
        instance.setDebugName(device.vk(), type, handle, name);
    }

private:
    // Context
    const Window &window;
    const VulkanInstance instance;
    VulkanSurface surface;
    const VulkanDevice device;
    VulkanSwapChain swapChain;
    // Command management
    const VulkanCommandPool graphicsCommandPool;
    const std::vector<VulkanCommandBuffer> primaryCommandBuffers;
    const VulkanCommandPool transferCommandPool;
    // Resources
    const VulkanMemory memory;
    const VulkanFrame frame;

    uint32_t currentFrame = 0;
    uint32_t currentSwapChainImage = 0;
    uint32_t currentFrameCounter = 0;
    std::deque<BufferedGPUResourceEntry> bufferedResourceDestroyQueue;
};

