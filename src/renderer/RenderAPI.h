#pragma once

#include <cstdint>

#include <glm/glm.hpp>

/// Defines the orchestration interface for renderers in this engine.
class RenderAPI {
public:
    // Wrappers for render object references for type safety
    using handle_type = uint32_t;

    struct RenderPassHandle {
        handle_type ref;
    };
    struct PipelineHandle {
        handle_type ref;
    };

    struct VertexArrayHandle {
        handle_type ref;
    };

    struct DescriptorHandle {
        handle_type ref;
    };

    struct ResourceHandle {
        handle_type ref;
    };

    struct BufferHandle : public ResourceHandle {
    };
    struct TextureHandle : public ResourceHandle {
    };

    struct ViewportDimensions {
        glm::vec2 offset;
        glm::vec2 dimensions;
    };

public:
    virtual ~RenderAPI();

    // Lifecycle
    /// Wait for GPU tasks to finish
    virtual void join() = 0;

    // Resource commands
    virtual RenderPassHandle createRenderPass(/*TODO*/) = 0;

    virtual VertexArrayHandle createVertexArray(/*TODO*/) = 0;

    virtual PipelineHandle createPipeline(/*TODO*/) = 0;

    virtual DescriptorHandle createDescriptor(/*TODO*/) = 0;

    virtual ResourceHandle createBuffer(/*TODO*/) = 0;

    virtual ResourceHandle createTexture(/*TODO*/) = 0;

    // Resource update commands

    virtual void writeVertexArray(VertexArrayHandle vertexArrayHandle, char *data, size_t size) = 0;

    virtual void writeDescriptor(std::initializer_list<ResourceHandle> ressources) = 0;

    virtual void writeBuffer(BufferHandle bufferHandle, char *data, size_t size) = 0;

    virtual void writeTexture(TextureHandle textureHandle, char *data, size_t size) = 0;

    // Context commands
    /// Start recording draw commands
    virtual void beginScene() = 0;

    /// Start rendering to the specified render pass
    virtual void beginRenderPass(RenderPassHandle renderPassHandle, ViewportDimensions viewportDimensions) = 0;

    /// Stop rendering to this render pass
    virtual void endRenderPass(RenderPassHandle renderPassHandle) = 0;

    /// Defines the shader to use for the next draw commands
    virtual void bindPipeline(PipelineHandle pipelineHandle) = 0;

    /// Defines the descriptor used to render the next draw commands
    virtual void bindDescriptor(DescriptorHandle descriptorHandle) = 0;

    /// Stop recording draw commands
    virtual void endScene() = 0;

    /// Render the ImGui specific render pass (ImGui requires some special setup per render apir)
    virtual void renderImGui() = 0;

    /// Submit the recorded commands to the gpu
    virtual void flush() = 0;

    // Rendering commands

    /// Render an object from its vertex array
    virtual void drawIndexed(VertexArrayHandle vertexArrayHandle) = 0;
};

