#pragma once

#include "Engine/src/renderer/window/Window.h"
#include "Engine/src/renderer/vulkan/command/VulkanCommandBuffer.h"

#include <vector>

class VulkanContext;

/**
 * This class handles the synchronization between frames and queue submission.
 */
class VulkanFrame {
private:
    VulkanFrame(Window &window, const VulkanContext &context, std::vector<VkSemaphore> &&imageAvailableSemaphores,
                std::vector<VkSemaphore> &&renderFinishedSemaphores, std::vector<VkFence> &&inFlightFences);

    void destroy();

public:
    ~VulkanFrame();

    VulkanFrame(const VulkanFrame &o) = delete;

    VulkanFrame &operator=(const VulkanFrame &o) = delete;

    VulkanFrame(VulkanFrame &&o) noexcept;

    VulkanFrame &operator=(VulkanFrame &&o) = delete;

    static VulkanFrame Create(Window &window, const VulkanContext &context, uint32_t maxFramesInFlight);

    /**
     * Submit the current command buffer to the GPU for rendering on the graphics queue.
     * @returns <b>false</b> if the submission failed and the swapchain resources need to be recreated <b>true</b> otherwise
     */
    [[nodiscard]] bool render(size_t currentFrame, const VulkanCommandBuffer &commandBuffer) const;

    /**
     * Wait until the current has finished rendering on the GPU and is free to be recorded again.
     * @param currentFrame
     */
    void waitUntilCurrentFrameIsFree(uint32_t currentFrame) const;

private:
    Window &window;
    const VulkanContext &context;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

};

