#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan usese 0 to 1
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <array>
#include <vector>
#include <unordered_map>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;

    bool operator==(const Vertex &o) const {
        return pos == o.pos &&
               color == o.color &&
               normal == o.normal &&
               uv == o.uv;
    }

    /* Defines how vertex data is loaded to the vertex shader*/
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0; // binding used by attributes
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // increment after vertex instead of after instance

        return bindingDescription;
    }

    /* Defines data layout of vertex data per vertex shader instance */
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0; // binding of InputBindingDescription
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0; // binding of InputBindingDescription
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0; // binding of InputBindingDescription
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, normal);

        attributeDescriptions[3].binding = 0; // binding of InputBindingDescription
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, uv);

        return attributeDescriptions;
    }
};

// Needed to enable Vertex instances as keys in unordered_maps
namespace std {
    template<>
    struct hash<Vertex> {
        size_t operator()(Vertex const &vertex) const noexcept;
    };
}


struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};