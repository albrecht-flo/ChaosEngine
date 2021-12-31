#pragma once

#include "Engine/src/renderer/vulkan/memory/VulkanBuffer.h"
#include "Engine/src/renderer/api/RenderMesh.h"

class VulkanVertexInput;

class VulkanRenderMesh : public Renderer::RenderMesh {
public:
    VulkanRenderMesh(VulkanBuffer &&vertexBuffer, VulkanBuffer &&indexBuffer, uint32_t indexCount)
            : RenderMesh(indexCount), vertexBuffer(std::move(vertexBuffer)), indexBuffer(std::move(indexBuffer)) {}

    ~VulkanRenderMesh() override = default;

    [[nodiscard]] inline const VulkanBuffer &getVertexBuffer() const { return vertexBuffer; }

    [[nodiscard]] inline const VulkanBuffer &getIndexBuffer() const { return indexBuffer; }

public:
    static const VulkanVertexInput vertex_3P_3C_3N_2U;
private:
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
};
