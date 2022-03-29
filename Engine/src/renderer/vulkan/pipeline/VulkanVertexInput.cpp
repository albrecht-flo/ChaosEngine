#include "VulkanVertexInput.h"

#include <utility>

#include <glm/glm.hpp>

VulkanVertexInput::VulkanVertexInput(std::vector<VkVertexInputBindingDescription> &&bindingDescription,
                                     std::vector<VkVertexInputAttributeDescription> &&attributeDescriptions)
        : bindingDescription(std::move(bindingDescription)), attributeDescriptions(std::move(attributeDescriptions)) {}

VertexAttributeBuilder &
VertexAttributeBuilder::addAttribute(uint32_t location, Renderer::VertexFormat format, uint32_t offset) {
    attributes.emplace_back(VkVertexInputAttributeDescription{
            .location = location,
            .binding = binding,
            .format = getVkFormat(format),
            .offset = offset}
    );
    return *this;
}

VulkanVertexInput VertexAttributeBuilder::build() {
    return VulkanVertexInput{{VkVertexInputBindingDescription{binding, stride, getVkInputRate(inputRate)}},
                             std::move(attributes)};
}

VkVertexInputRate VertexAttributeBuilder::getVkInputRate(Renderer::InputRate inputRate) {
    switch (inputRate) {
        case Renderer::InputRate::Vertex:
            return VK_VERTEX_INPUT_RATE_VERTEX;
        case Renderer::InputRate::Instance:
            return VK_VERTEX_INPUT_RATE_INSTANCE;
    }
    assert("Unknown Vertex Input Rate" && false);
    return VK_VERTEX_INPUT_RATE_VERTEX;
}

VkFormat VertexAttributeBuilder::getVkFormat(Renderer::VertexFormat format) {
    using namespace Renderer;
    switch (format) {
        case VertexFormat::R_FLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case VertexFormat::RG_FLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case VertexFormat::RGB_FLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexFormat::RGBA_FLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    assert("Unknown Vertex Format" && false);
    return VK_FORMAT_R32_SFLOAT;
}
