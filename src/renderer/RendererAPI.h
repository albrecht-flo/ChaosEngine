#pragma once

#include <cstdint>

#include <glm/glm.hpp>

/// Defines the orchestration interface for renderers in this engine.
class RendererAPI {
public:
    // Wrappers for render object references for type safety
    using ref_type = uint32_t;

    struct ShaderRef {
        ref_type ref;
    };

    struct MeshRef {
        ref_type ref;
    };

    struct MaterialRef {
        ref_type ref;
    };

public:
    virtual ~RendererAPI();

    // Lifecycle
    /// Setup for all dynamic resources
    virtual void setup() = 0;
    /// Wait for GPU tasks to finish
    virtual void join() = 0;

    // Context commands
    /// Start recording commands with this renderer
    virtual void beginScene(/*environment, camera*/) = 0;

    /// Define the shader to user for the next render commands
    virtual void useShader(ShaderRef shaderRef) = 0;

    /// Stop recording commands with this renderer
    virtual void endScene(/*Post Processing config*/) = 0;

    /// Submit recorded commands to gpu
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

