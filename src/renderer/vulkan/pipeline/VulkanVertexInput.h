#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanVertexInput {
public:
    static VulkanVertexInput Vertex_3_3_3_2;
public:
    VulkanVertexInput() = default;

    VulkanVertexInput(std::vector<VkVertexInputBindingDescription> &&bindingDescription,
                      std::vector<VkVertexInputAttributeDescription> &&attributeDescriptions);

    ~VulkanVertexInput() = default;

    VulkanVertexInput(const VulkanVertexInput &o) = default;

    VulkanVertexInput &operator=(const VulkanVertexInput &o) = default;

    VulkanVertexInput(VulkanVertexInput &&o) = default;

    VulkanVertexInput &operator=(VulkanVertexInput &&o) = default;

    [[nodiscard]] inline const std::vector<VkVertexInputAttributeDescription> &
    getAttributeDescriptions() const { return attributeDescriptions; }

    [[nodiscard]] inline const std::vector<VkVertexInputBindingDescription> &
    getBindingDescription() const { return bindingDescription; }

private:
    std::vector<VkVertexInputBindingDescription> bindingDescription;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

enum class InputRate {
    Vertex, Instance
};

class VertexAttributeBuilder {
public:
    VertexAttributeBuilder(uint32_t binding, uint32_t stride, InputRate inputRate)
            : binding(binding), stride(stride), inputRate(inputRate), attributes() {}

    VertexAttributeBuilder &addAttribute(uint32_t location, VkFormat format, uint32_t offset) {
        attributes.emplace_back(VkVertexInputAttributeDescription{binding, location, format, offset});
        return *this;
    }

    VulkanVertexInput build() {
        auto vInputRate = (inputRate == InputRate::Vertex) ? VK_VERTEX_INPUT_RATE_VERTEX
                                                           : VK_VERTEX_INPUT_RATE_INSTANCE;
        return VulkanVertexInput{{VkVertexInputBindingDescription{binding, stride, vInputRate}},
                                 std::move(attributes)};
    }

private:
    uint32_t binding;
    uint32_t stride;
    InputRate inputRate;
    std::vector<VkVertexInputAttributeDescription> attributes;
};