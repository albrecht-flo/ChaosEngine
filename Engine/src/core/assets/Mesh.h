#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan usese 0 to 1
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>
#include <unordered_map>

struct VertexPNCU {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;

    bool operator==(const VertexPNCU &o) const {
        return pos == o.pos && color == o.color && normal == o.normal && uv == o.uv;
    };
};

struct VertexPCU {
    glm::vec3 pos;
    glm::vec4 color;
    glm::vec2 uv;

    bool operator==(const VertexPCU &o) const {
        return pos == o.pos && color == o.color && uv == o.uv;
    };
};

// Needed to enable Vertex instances as keys in unordered_maps
namespace std {
    template<>
    struct hash<VertexPNCU> {
        size_t operator()(VertexPNCU const &vertex) const noexcept;
    };

    template<>
    struct hash<VertexPCU> {
        size_t operator()(VertexPCU const &vertex) const noexcept;
    };
}

struct MeshPNCU {
    std::vector<VertexPNCU> vertices;
    std::vector<uint32_t> indices;
};

struct MeshPCU {
    std::vector<VertexPCU> vertices;
    std::vector<uint32_t> indices;
};

struct LightObject {
    glm::vec4 lightPos; // w = lightRadius
    glm::vec4 lightColor; // w = ambient ammount
};
