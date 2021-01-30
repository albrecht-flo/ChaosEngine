#pragma once

#include <src/renderer/vulkan/context/VulkanContext.h>
#include "src/renderer/RendererAPI.h"

class VulkanRenderer2D : public RendererAPI {
public:
    static constexpr uint32_t maxFramesInFlight = 2;
private:
    VulkanRenderer2D(VulkanContext &&context, VulkanFrame &&frame);

public:
    ~VulkanRenderer2D() override = default;

    VulkanRenderer2D(const VulkanRenderer2D &o) = delete;

    VulkanRenderer2D &operator=(const VulkanRenderer2D &o) = delete;

    VulkanRenderer2D(VulkanRenderer2D &&o) noexcept;

    VulkanRenderer2D &operator=(VulkanRenderer2D &&o) = delete;

    static VulkanRenderer2D Create(Window &window);

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

private:
    VulkanContext context;
    VulkanFrame frame;
    std::vector<VulkanCommandBuffer> primaryCommandBuffers;

    uint32_t currentFrame = 0;
};
