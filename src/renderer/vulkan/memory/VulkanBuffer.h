#pragma once

#include <memory>

struct VulkanBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
};

struct VulkanUniformBuffer : public VulkanBuffer {
    VkDeviceSize size;
    VkDeviceSize alignment;
};

/* Not nearly done, TODO: refactor, INSECURE! */
template<typename T>
class UniformBufferContent {
public:
    UniformBufferContent(uint32_t elementCount = 0, uint32_t alignment = 0) :
            m_data(nullptr), m_size(elementCount * sizeof(T)), m_alignment(alignment) {
        if (m_size != 0) {
            if (m_alignment == 0)
                m_data = malloc(m_size);
            else {
#ifdef _MSC_VER // MS Visual studio does not include aligned_alloc
                m_data = _aligned_malloc((uint32_t) m_size, (uint32_t) m_alignment);
#else
                m_data = std::aligned_alloc((uint32_t)m_size, (uint32_t)m_alignment);
#endif
            }
        }
    }

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

    void *data() const { return m_data; }

    uint32_t size() const { return m_size; }

    uint32_t alignment() const { return m_alignment; }

private:
    void *m_data = nullptr;
    uint32_t m_size = 0;
    uint32_t m_alignment = 0;
};
