#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Engine/src/renderer/api/Material.h"

class VulkanVertexInput {
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

class VertexAttributeBuilder {
public:
    VertexAttributeBuilder(uint32_t binding, uint32_t stride, Renderer::InputRate inputRate)
            : binding(binding), stride(stride), inputRate(inputRate), attributes() {}

    VertexAttributeBuilder &addAttribute(uint32_t location, Renderer::VertexFormat format, uint32_t offset);

    VulkanVertexInput build();

private:
    uint32_t binding;
    uint32_t stride;
    Renderer::InputRate inputRate;
    std::vector<VkVertexInputAttributeDescription> attributes;
private:
    // ------------------------------------ Translation Helpers --------------------------------------------------------
    static VkFormat getVkFormat(Renderer::VertexFormat format);

    static VkVertexInputRate getVkInputRate(Renderer::InputRate inputRate);
};