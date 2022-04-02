#pragma once

#include "Engine/src/renderer/vulkan/memory/VulkanBuffer.h"
#include "Engine/src/renderer/api/RenderMesh.h"

class VulkanVertexInput;

class VulkanRenderMesh : public Renderer::RenderMesh {
public:
    VulkanRenderMesh(VulkanBuffer &&vertexBuffer, VulkanBuffer &&indexBuffer, uint32_t indexCount)
            : RenderMesh(indexCount), vertexBuffer(std::move(vertexBuffer)), indexBuffer(std::move(indexBuffer)) {}

    ~VulkanRenderMesh() override = default;

    [[nodiscard]] const Renderer::Buffer *getVertexBuffer() const override { return &vertexBuffer; }

    [[nodiscard]] const Renderer::Buffer *getIndexBuffer() const override { return &indexBuffer; }

public:
    static const VulkanVertexInput vertex_3P_3C_3N_2U;
private:
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
};
