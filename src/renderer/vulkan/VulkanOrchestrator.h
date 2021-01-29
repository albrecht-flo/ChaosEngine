#pragma once

#include <src/renderer/vulkan/context/VulkanContext.h>
#include "src/renderer/RendererAPI.h"
#include "src/renderer/window/Window.h"

class VulkanOrchestrator : public RendererAPI {
private:
    VulkanOrchestrator(Window &window, VulkanContext&& context);
public:
    ~VulkanOrchestrator() override = default;

    VulkanOrchestrator Create(Window &window);

    // Lifecycle

    /// Wait for GPU tasks to finish
    void join() override;

    // Context commands
    /// Start recording commands with this renderer
    void beginScene(/*environment, camera*/) override;

    /// Define the shader to user for the next render commands
    void useShader(ShaderRef shaderRef) override;

    /// Stop recording commands with this renderer
    void endScene(/*Post Processing config*/) override;

    /// Submit recorded commands to gpu
    void flush() override;

    // Rendering commands
    /// Render an object with its material and model matrix
    void renderObject(MeshRef meshRef, MaterialRef materialRef, glm::mat4 modelMat) override;

    // Data upload commands
    /*** Load a shader program from disk and upload it to the GPU.
     *
     * @return A reference to the created shader program.
     */
    ShaderRef loadShader(/*Resource definition*/) override;

    /*** Load a mesh from disk and upload it to the GPU.
     *
     * @return A reference to the created mesh.
     */
    MeshRef loadMesh(/*Resource definition*/) override;

    /*** Load a material from disk and upload it to the GPU.
     *
     * @return A reference to the created material.
     */
    MaterialRef loadMaterial(/*Resource definition*/) override;

private: // Fields
    VulkanContext context;
};
