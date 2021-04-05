#include "VulkanRenderMesh.h"
#include "Engine/src/core/assets/Mesh.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanVertexInput.h"

const VulkanVertexInput VulkanRenderMesh::vertex_3P_3C_3N_2U =
        VertexAttributeBuilder(0, sizeof(Vertex), InputRate::Vertex)
                .addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
                .addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
                .addAttribute(2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
                .addAttribute(3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)).build();