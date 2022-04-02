#include "Mesh.h"


// Adapted from Boost: https://www.boost.org/LICENSE_1_0.txt
inline void hash_combine(std::size_t &seed, size_t hash) {
    seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// To enable Vertex as a key in a unordered map
// Source: https://vulkan-tutorial.com/Loading_models
namespace std {
    size_t hash<VertexPNCU>::operator()(VertexPNCU const &vertex) const noexcept {
        size_t value = 0;
        hash_combine(value, hash<glm::vec3>()(vertex.pos));
        hash_combine(value, hash<glm::vec3>()(vertex.normal));
        hash_combine(value, hash<glm::vec3>()(vertex.color));
        hash_combine(value, hash<glm::vec2>()(vertex.uv));
        return value;
    }

    size_t hash<VertexPCU>::operator()(VertexPCU const &vertex) const noexcept {
        size_t value = 0;
        hash_combine(value, hash<glm::vec3>()(vertex.pos));
        hash_combine(value, hash<glm::vec4>()(vertex.color));
        hash_combine(value, hash<glm::vec2>()(vertex.uv));
        return value;
    }
}
