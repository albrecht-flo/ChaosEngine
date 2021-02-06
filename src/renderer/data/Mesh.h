#pragma once

#include <vulkan/vulkan.h>

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
        return pos == o.pos && color == o.color && normal == o.normal && uv == o.uv;
    };
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