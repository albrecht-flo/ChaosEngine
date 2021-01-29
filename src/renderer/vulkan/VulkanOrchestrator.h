#pragma once

#include "src/renderer/vulkan/context/VulkanContext.h"
#include "src/renderer/vulkan/memory/VulkanMemory.h"
#include "src/renderer/RendererAPI.h"

class Window;

class VulkanCommandBuffer;

/// This class orchestrates the creation, recreation and communication of the all vulkan objects.
class VulkanOrchestrator : public RendererAPI {
private:
    VulkanOrchestrator(VulkanContext &&context, VulkanSwapChain &&swapChain,
                       std::vector<VulkanCommandBuffer> &&primaryCommandBuffers);

public:
    ~VulkanOrchestrator() override = default;

    VulkanOrchestrator(const VulkanOrchestrator &o) = delete;

    VulkanOrchestrator &operator=(const VulkanOrchestrator &o) = delete;

    VulkanOrchestrator(VulkanOrchestrator &&o) = delete;

    VulkanOrchestrator &operator=(VulkanOrchestrator &&o) = delete;

    static VulkanOrchestrator Create(const Window &window);

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
    VulkanSwapChain swapChain;

    std::vector<VulkanCommandBuffer> primaryCommandBuffers;
};
