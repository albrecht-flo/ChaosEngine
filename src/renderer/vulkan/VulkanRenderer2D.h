#pragma once

#include <src/renderer/vulkan/image/VulkanImage.h>
#include "src/renderer/RendererAPI.h"
#include "src/renderer/vulkan/rendering/VulkanFrame.h"
#include "src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "src/renderer/vulkan/context/VulkanContext.h"
#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "VulkanDataManager.h"

class VulkanRenderer2D : public RendererAPI {
private:
    struct CameraUbo {
        glm::mat4 view;
        glm::mat4 proj;
    };

    static constexpr uint32_t maxFramesInFlight = 2;
private:
    VulkanRenderer2D(VulkanContext &&context, VulkanFrame &&frame,
                     std::vector<VulkanFramebuffer> &&swapChainFrameBuffers,
                     VulkanRenderPass &&mainRenderPass, VulkanImageBuffer &&depthBuffer);

public:
    ~VulkanRenderer2D() override = default;

    VulkanRenderer2D(const VulkanRenderer2D &o) = delete;

    VulkanRenderer2D &operator=(const VulkanRenderer2D &o) = delete;

    VulkanRenderer2D(VulkanRenderer2D &&o) = delete;

    VulkanRenderer2D &operator=(VulkanRenderer2D &&o) = delete;

    static VulkanRenderer2D Create(Window &window);

    // Lifecycle
    /// Setup for all dynamic resources
    void setup() override;

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
    void recreateSwapChain();

private:
    VulkanContext context;
    VulkanFrame frame;
    std::vector<VulkanCommandBuffer> primaryCommandBuffers;
    std::vector<VulkanFramebuffer> swapChainFrameBuffers;
    VulkanRenderPass mainRenderPass;

    VulkanImageBuffer depthBuffer;

    VulkanDataManager pipelineManager;
    VulkanMemory vulkanMemory;

    uint32_t currentFrame = 0;

    // Dynamic resources -----------------------------------------------------
    std::unique_ptr<VulkanVertexInput> vertex_3P_3C_3N_2U;
    std::unique_ptr<VulkanDescriptorSetLayout> cameraDescriptorLayout;
    std::unique_ptr<VulkanDescriptorSetLayout> materialDescriptorLayout;
    std::unique_ptr<VulkanPipeline> pipeline;
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;

    // ----- Per Frame resources
    std::vector<VulkanDescriptorSet> perFrameDescriptorSets;
    std::vector<VulkanUniformBuffer> perFrameUniformBuffers;
};

