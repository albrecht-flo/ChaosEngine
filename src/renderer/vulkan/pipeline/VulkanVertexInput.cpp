#include "VulkanVertexInput.h"

#include <utility>

#include <glm/glm.hpp>

VulkanVertexInput::VulkanVertexInput(std::vector<VkVertexInputBindingDescription> &&bindingDescription,
                                     std::vector<VkVertexInputAttributeDescription> &&attributeDescriptions)
        : attributeDescriptions(std::move(attributeDescriptions)), bindingDescription(std::move(bindingDescription)) {}


struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;
};

VulkanVertexInput VulkanVertexInput::Vertex_3_3_3_2 = VertexAttributeBuilder(0, sizeof(Vertex), InputRate::Vertex)
        .addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
        .addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
        .addAttribute(2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
        .addAttribute(3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)).build();