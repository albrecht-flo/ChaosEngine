#pragma once

#include "renderer/api/Buffer.h"
#include "renderer/api/BufferedGPUResource.h"
#include "core/RenderingSystem.h"

#include "renderer/vulkan/context/VulkanContext.h"
#include "VulkanMemory.h"

#include <memory>

class VulkanBuffer : public Renderer::Buffer {
    friend class VulkanMemory;

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
        destroy();
        buffer = std::exchange(o.buffer, nullptr);
        allocation = std::exchange(o.allocation, nullptr);
        return *this;
    }

    void destroyImmediately() {
        memory.destroyBuffer(buffer, allocation);
        buffer = nullptr;
        allocation = nullptr;
    }

    [[nodiscard]] inline VkBuffer vk() const { return buffer; }

private:
    void destroy() {
        if (buffer != nullptr) {
            auto &vulkanContext = dynamic_cast<VulkanContext &>(RenderingSystem::GetContext());
            vulkanContext.destroyBuffered(std::make_unique<VulkanBufferBufferedDestroy>(memory, buffer, allocation));
            buffer = nullptr;
            allocation = nullptr;
        }
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


    inline void destroyImmediately() { buffer.destroyImmediately(); }

    [[nodiscard]] inline const VulkanBuffer &getBuffer() const { return buffer; }

    [[nodiscard]] inline uint64_t getSize() const { return size; }

private:
    VulkanBuffer buffer;
    uint64_t size;
    uint64_t alignment;
};
