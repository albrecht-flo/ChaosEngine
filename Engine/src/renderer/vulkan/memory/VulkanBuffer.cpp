#include "VulkanBuffer.h"

void *VulkanBuffer::map() {
    // TODO: This will fail for GPU_ONLY buffers!
    if (mapping == nullptr)
        mapping = memory.mapBuffer(allocation);
    return mapping;
}

void VulkanBuffer::flush() {
    // TODO: This is only necessary for GPU_ONLY buffers! NOT yet implemented
}

void VulkanBuffer::unmap() {
    memory.unmapBuffer(allocation);
    mapping = nullptr;
}

void VulkanBuffer::copy(void *data, size_t bytes) {
    memory.copyDataToBuffer(*this, data, bytes, 0);
}

void VulkanBuffer::destroy() {
    if (buffer != 0) {
        if (mapping != nullptr) {
            unmap();
        }
        auto &vulkanContext = dynamic_cast<VulkanContext &>(ChaosEngine::RenderingSystem::GetContext());
        vulkanContext.destroyBuffered(std::make_unique<VulkanBufferBufferedDestroy>(memory, buffer, allocation));
        buffer = 0;
        allocation = nullptr;
    }
}

void VulkanBuffer::destroyImmediately() {
    memory.destroyBuffer(buffer, allocation);
    buffer = 0;
    allocation = nullptr;
    mapping = nullptr;
}

// ----------------------------Vulkan Uniform Buffer -------------------------------------------------------------------
