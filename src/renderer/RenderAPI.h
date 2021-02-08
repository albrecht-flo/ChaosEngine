#pragma once

#include <cstdint>

#include <glm/glm.hpp>

/// Defines the orchestration interface for renderers in this engine.
class RendererAPI {
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

    struct ViewportDimensions {
        glm::vec2 offset;
        glm::vec2 dimensions;
    };

public:
    virtual ~RendererAPI();

    // Lifecycle
    /// Wait for GPU tasks to finish
    virtual void join() = 0;

    // Context commands
    /// Start recording draw commands
    virtual void beginScene() = 0;

    /// Start rendering to the specified render pass
    virtual void beginRenderPass(RenderPassHandle renderPassHandle, ViewportDimensions viewportDimensions)  = 0;

    /// Stop rendering to this render pass
    virtual void endRenderPass(RenderPassHandle renderPassHandle) = 0;

    /// Define the shader to user for the next render commands
    virtual void bindPipeline(PipelineHandle pipelineHandle) = 0;

    /// Stop recording draw commands
    virtual void endScene() = 0;

    /// Render the ImGui specific render pass (ImGui requires some special setup per render apir)
    virtual void renderImGui() = 0;

    /// Submit the recorded commands to the gpu
    virtual void flush() = 0;

    // Rendering commands

    /// Render an object with its material and model matrix
    virtual void renderObject(MeshRef meshRef, MaterialRef materialRef, glm::mat4 modelMat) = 0;

    // Data upload commands
    /*** Load a mesh from disk and upload it to the GPU.
     *
     * @return A reference to the created mesh.
     */
    virtual MeshRef loadMesh(/*Resource definition*/) = 0;

    /*** Load a material from disk and upload it to the GPU.
     *
     * @return A reference to the created material.
     */
    virtual MaterialRef loadMaterial(/*Resource definition*/) = 0;
};

