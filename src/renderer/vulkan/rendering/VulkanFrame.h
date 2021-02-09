#pragma once

#include <vulkan/vulkan.h>

#include "src/renderer/window/Window.h"
#include "src/renderer/vulkan/command/VulkanCommandBuffer.h"

#include <vector>

class VulkanContext;

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

    [[nodiscard]] bool render(size_t currentFrame, const VulkanCommandBuffer &commandBuffer) const;

private:
    Window &window;
    const VulkanContext &context;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

};

