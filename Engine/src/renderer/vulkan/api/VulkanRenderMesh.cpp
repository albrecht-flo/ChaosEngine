#include "VulkanRenderMesh.h"
#include "Engine/src/core/assets/Mesh.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanVertexInput.h"

using namespace Renderer;

const VulkanVertexInput VulkanRenderMesh::vertex_3P_3C_3N_2U =
        VertexAttributeBuilder(0, sizeof(Vertex), Renderer::InputRate::Vertex)
                .addAttribute(0, VertexFormat::RGB_FLOAT, offsetof(Vertex, pos))
                .addAttribute(1, VertexFormat::RGB_FLOAT, offsetof(Vertex, color))
                .addAttribute(2, VertexFormat::RGB_FLOAT, offsetof(Vertex, normal))
                .addAttribute(3, VertexFormat::RG_FLOAT, offsetof(Vertex, uv)
                ).build();