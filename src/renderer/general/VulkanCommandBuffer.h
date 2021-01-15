#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "../general/VulkanDevice.h"

/* Contains a single command buffer. 
	TOBE: Factory to bulk create command buffers.
		Grouping command buffers
*/
class VulkanCommandBuffer {
public:
    VulkanCommandBuffer(VulkanDevice &device, VkCommandPool commandPool, VkCommandBufferLevel level);

    ~VulkanCommandBuffer();

    void begin(VkCommandBufferUsageFlags flags);

    void end();

    void destroy();

    VkCommandBuffer getBuffer() { return m_buffer; }

private:
    VulkanDevice &m_device;
    VkCommandPool m_commandPool;
    VkCommandBuffer m_buffer;
};

