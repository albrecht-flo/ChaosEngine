#pragma once

#include "renderer/api/Buffer.h"
#include "renderer/api/BufferedGPUResource.h"
#include "Engine/src/core/renderSystem/RenderingSystem.h"

#include "renderer/vulkan/context/VulkanContext.h"
#include "VulkanMemory.h"

#include <memory>

class VulkanBuffer : public Renderer::Buffer {
    friend VulkanMemory;

    /**
     * This class handles the buffered destruction of a VulkanBuffer.
     */
    class VulkanBufferBufferedDestroy : public BufferedGPUResource {
    public:
        VulkanBufferBufferedDestroy(const VulkanMemory &memory, VkBuffer buffer, VmaAllocation allocation)
                : memory(memory), buffer(buffer), allocation(allocation) {}

        ~VulkanBufferBufferedDestroy() override = default;

        void destroy() override {
            memory.destroyBuffer(buffer, allocation);
        }

        [[nodiscard]] std::string toString() const override {
            char str[17];
            snprintf(str, sizeof(str), "%p", (void *) buffer);
            return "VulkanBuffer " + std::string(str);
        }

    private:
        const VulkanMemory &memory;
        VkBuffer buffer;
        VmaAllocation allocation;
    };

public:
    VulkanBuffer(const VulkanMemory &memory, VkBuffer buffer, VmaAllocation allocation)
            : Renderer::Buffer(), memory(memory), buffer(buffer), allocation(allocation), mapping(nullptr) {}

    ~VulkanBuffer() override { destroy(); }

    VulkanBuffer(const VulkanBuffer &o) = delete;

    VulkanBuffer &operator=(const VulkanBuffer &o) = delete;

    VulkanBuffer(VulkanBuffer &&o) noexcept
            : Renderer::Buffer(), memory(o.memory), buffer(std::exchange(o.buffer, nullptr)),
              allocation(std::exchange(o.allocation, nullptr)), mapping(std::exchange(o.mapping, nullptr)) {}

    VulkanBuffer &operator=(VulkanBuffer &&o) noexcept {
        if (&o == this)
            return *this;
        destroy();
        buffer = std::exchange(o.buffer, nullptr);
        allocation = std::exchange(o.allocation, nullptr);
        mapping = std::exchange(o.mapping, nullptr);
        return *this;
    }

    void destroyImmediately();

    [[nodiscard]] inline VkBuffer vk() const { return buffer; }

    void *map() override;

    void flush() override;

    void unmap() override;

    void copy(void *data, size_t bytes) override;

private:
    void destroy();

private:
    const VulkanMemory &memory;
    VkBuffer buffer;
    VmaAllocation allocation;
    void *mapping;
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

    inline void destroyImmediately() { buffer.destroyImmediately(); }

    void *map() override { return buffer.map(); }

    void flush() override { buffer.flush(); }

    void unmap() override { buffer.unmap(); }

    void copy(void *data, size_t bytes) override { buffer.copy(data, bytes); }

    [[nodiscard]] inline const VulkanBuffer &getBuffer() const { return buffer; }

    [[nodiscard]] inline uint64_t getSize() const { return size; }

private:
    VulkanBuffer buffer;
    uint64_t size;
    uint64_t alignment;
};
