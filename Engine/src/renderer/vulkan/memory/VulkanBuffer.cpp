#include "VulkanBuffer.h"

void *VulkanBuffer::map() {
    // TODO: This will fail for GPU_ONLY buffers!
    if (mapping == nullptr)
        mapping = memory.mapBuffer(allocation);
    return mapping;
}

void VulkanBuffer::flush() {
    // TODO: This is only necessary for GPU_ONLY buffers!
}

void VulkanBuffer::unmap() {
    memory.unmapBuffer(allocation);
    mapping = nullptr;
}

void VulkanBuffer::copy(size_t bytes) {
    memory.copyDataToBuffer(*this, mapping, bytes, 0);
}
