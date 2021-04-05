#include "Mesh.h"

// To enable Vertex as a key in a unordered map
// Source: https://vulkan-tutorial.com/Loading_models
namespace std {
    size_t hash<Vertex>::operator()(Vertex const &vertex) const noexcept {
        return ((hash<glm::vec3>()(vertex.pos) ^
                 (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
               (hash<glm::vec2>()(vertex.uv) << 1);
    }
}
