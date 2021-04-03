#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <memory>

/*
 * TODO: refactor, [Part of VulkanMemory refactoring]
 * Known: Memory Leak
 */

class VulkanBuffer {
    friend class VulkanMemory;

public:
    VulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation)
            : allocator(allocator), buffer(buffer), allocation(allocation) {}

    virtual ~VulkanBuffer() { destroy(); }

    VulkanBuffer(const VulkanBuffer &o) = delete;

    VulkanBuffer &operator=(const VulkanBuffer &o) = delete;

    VulkanBuffer(VulkanBuffer &&o) noexcept
            : allocator(std::exchange(o.allocator, nullptr)), buffer(std::exchange(o.buffer, nullptr)),
              allocation(std::exchange(o.allocation, nullptr)) {}

    VulkanBuffer &operator=(VulkanBuffer &&o) noexcept {
        if (&o == this)
            return *this;
        allocator = std::exchange(o.allocator, nullptr);
        buffer = std::exchange(o.buffer, nullptr);
        allocation = std::exchange(o.allocation, nullptr);
        return *this;
    }

    [[nodiscard]] inline VkBuffer vk() const { return buffer; }

private:
    void destroy() {
        if (allocator != nullptr)
            vmaDestroyBuffer(allocator, buffer, allocation);
    }

private:
    VmaAllocator allocator;
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct VulkanUniformBuffer {
public:
    VulkanUniformBuffer(VulkanBuffer &&buffer, uint64_t size, uint64_t alignment)
            : buffer(std::move(buffer)), size(size), alignment(alignment) {}

    ~VulkanUniformBuffer() = default;

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

template<typename T>
class UniformBufferContent {
public:
    explicit UniformBufferContent(uint32_t elementCount, uint32_t alignment = 0) :
            m_data(nullptr), m_size(elementCount * sizeof(T)), m_alignment(alignment) {
        if (m_alignment == 0)
            m_data = malloc(m_size);
        else {
#ifdef _MSC_VER // MS Visual studio does not include aligned_alloc
            m_data = _aligned_malloc((size_t) m_size, (size_t) m_alignment);
#else
            m_data = std::aligned_alloc((size_t) m_size, (size_t) m_alignment);
#endif
        }
    }

    ~UniformBufferContent() { destroy(); }

    UniformBufferContent(const UniformBufferContent &o) = delete;

    UniformBufferContent &operator=(const UniformBufferContent &o) = delete;

    UniformBufferContent(UniformBufferContent &&o) noexcept
            : m_data(std::exchange(o.m_data, nullptr)), m_size(std::exchange(o.m_size, 0)),
              m_alignment(std::exchange(o.m_alignment, 0)) {}

    UniformBufferContent &operator=(UniformBufferContent &&o) noexcept {
        if (this == &o)
            return *this;
        destroy();
        m_data = std::exchange(o.m_data, nullptr);
        m_size = std::exchange(o.m_size, 0);
        m_alignment = std::exchange(o.m_alignment, 0);
        return *this;
    }


    void destroy() {
        if (m_data != nullptr) {
            if (m_alignment == 0)
                free(m_data);
            else {
#ifdef _MSC_VER
                _aligned_free(m_data);
#else
                std::free(m_data);
#endif
            }
        }
    }

    T *at(uint32_t i = 0) {
        return (T *) (((char *) m_data) + (i * m_alignment));
    }

    [[nodiscard]] void *data() const { return m_data; }

    [[nodiscard]] uint32_t size() const { return m_size; }

    [[nodiscard]] uint32_t alignment() const { return m_alignment; }

private:
    void *m_data = nullptr;
    uint32_t m_size = 0;
    uint32_t m_alignment = 0;
};
