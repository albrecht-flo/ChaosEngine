#pragma once

#include "Engine/src/renderer/api/Buffer.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <memory>

#include "VulkanMemory.h"

class VulkanBuffer : public Renderer::Buffer {
    friend class VulkanMemory;

public:
    VulkanBuffer(const VulkanMemory &memory, VkBuffer buffer, VmaAllocation allocation)
            : Renderer::Buffer(), memory(memory), buffer(buffer), allocation(allocation) {}

    ~VulkanBuffer() override { destroy(); }

    VulkanBuffer(const VulkanBuffer &o) = delete;

    VulkanBuffer &operator=(const VulkanBuffer &o) = delete;

    VulkanBuffer(VulkanBuffer &&o) noexcept
            : Renderer::Buffer(), memory(o.memory), buffer(std::exchange(o.buffer, nullptr)),
              allocation(std::exchange(o.allocation, nullptr)) {}

    VulkanBuffer &operator=(VulkanBuffer &&o) noexcept {
        if (&o == this)
            return *this;
        buffer = std::exchange(o.buffer, nullptr);
        allocation = std::exchange(o.allocation, nullptr);
        return *this;
    }

    [[nodiscard]] inline VkBuffer vk() const { return buffer; }

private:
    void destroy() {
        memory.destroyBuffer(buffer, allocation);
    }

private:
    const VulkanMemory &memory;
    VkBuffer buffer;
    VmaAllocation allocation;
};

class VulkanUniformBuffer : public Renderer::Buffer {
public:
    VulkanUniformBuffer(VulkanBuffer &&buffer, uint64_t size, uint64_t alignment)
            : buffer(std::move(buffer)), size(size), alignment(alignment) {}

    ~VulkanUniformBuffer() override = default;

    VulkanUniformBuffer(const VulkanUniformBuffer &o) = delete;

    VulkanUniformBuffer &operator=(const VulkanUniformBuffer &o) = delete;

    VulkanUniformBuffer(VulkanUniformBuffer &&o) noexcept
            : buffer(std::move(o.buffer)), size(o.size), alignment(o.alignment) {}

    VulkanUniformBuffer &operator=(VulkanUniformBuffer &&o) noexcept {
        if (&o == this)
            return *this;
        buffer = std::move(o.buffer);
        size = o.size;
        alignment = o.alignment;
        return *this;
    }

    [[nodiscard]] inline const VulkanBuffer &getBuffer() const { return buffer; }

    [[nodiscard]] inline uint64_t getSize() const { return size; }

private:
    VulkanBuffer buffer;
    uint64_t size;
    uint64_t alignment;
};
