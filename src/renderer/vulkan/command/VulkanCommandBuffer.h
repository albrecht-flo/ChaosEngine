#pragma once

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanCommandPool.h"

/* Contains a single command buffer. 
	TOBE: Factory to bulk create command buffers.
		Grouping command buffers
*/
class VulkanCommandBuffer {
private:
    VulkanCommandBuffer(const VulkanDevice &device, const VulkanCommandPool &commandPool, VkCommandBuffer buffer);

public:
    ~VulkanCommandBuffer();

    VulkanCommandBuffer(const VulkanCommandBuffer &o) = delete;

    VulkanCommandBuffer &operator=(const VulkanCommandBuffer &o) = delete;

    VulkanCommandBuffer(VulkanCommandBuffer &&o) noexcept;

    VulkanCommandBuffer &operator=(VulkanCommandBuffer &&o) = delete;

    static VulkanCommandBuffer
    Create(const VulkanDevice &device, const VulkanCommandPool &commandPool, VkCommandBufferLevel level);

    void begin(VkCommandBufferUsageFlags flags) const;

    void end() const;

    void destroy();

    [[nodiscard]] inline VkCommandBuffer vk() const { return buffer; }

private:
    const VulkanDevice &device;
    const VulkanCommandPool &commandPool;
    VkCommandBuffer buffer;
};

