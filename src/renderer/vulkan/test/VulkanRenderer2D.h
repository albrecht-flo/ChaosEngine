#pragma once


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan usese 0 to 1
#include <glm/glm.hpp>

#include <src/renderer/vulkan/image/VulkanImage.h>
#include <src/renderer/data/RenderObject.h>
#include <src/renderer/vulkan/pipeline/VulkanVertexInput.h>
#include "src/renderer/vulkan/rendering/VulkanFrame.h"
#include "src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "src/renderer/vulkan/context/VulkanContext.h"
#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "src/renderer/vulkan/pipeline/VulkanPipeline.h"

class VulkanRenderer2D {
private:
    /// Camera uniform object. Alignas 16 to ensure proper alignment with the GPU storage
    struct CameraUbo {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    static constexpr uint32_t maxFramesInFlight = 2;
private:
    VulkanRenderer2D(std::unique_ptr<VulkanContext> &&context, VulkanFrame &&frame,
                     std::vector<VulkanCommandBuffer> &&primaryCommandBuffers, VulkanRenderPass &&mainRenderPass,
                     VulkanImageBuffer &&depthBuffer, std::vector<VulkanFramebuffer> &&swapChainFrameBuffers);

public:
    ~VulkanRenderer2D() = default;

    VulkanRenderer2D(const VulkanRenderer2D &o) = delete;

    VulkanRenderer2D &operator=(const VulkanRenderer2D &o) = delete;

    VulkanRenderer2D(VulkanRenderer2D &&o) = delete;


    VulkanRenderer2D &operator=(VulkanRenderer2D &&o) = delete;

    static VulkanRenderer2D Create(Window &window);

    // Lifecycle
    /// Setup for all dynamic resources
    void setup();

    /// Wait for GPU tasks to finish
    void join();

    // Context commands
    /// Start recording commands with this renderer
    void beginScene(const glm::mat4 &cameraTransform);

    /// Stop recording commands with this renderer
    void endScene(/*Post Processing config*/);

    /// Submit recorded commands to gpu
    void flush();

    // Rendering commands
    /// Render an object with its material and model matrix
    void renderQuad(glm::mat4 modelMat, glm::vec4 color);


private:
    void recreateSwapChain();

    void updateUniformBuffer(glm::mat4 viewMat, glm::vec2 viewportDimensions);

private:
    std::unique_ptr<VulkanContext> context;
    VulkanFrame frame;
    std::vector<VulkanCommandBuffer> primaryCommandBuffers;
    std::vector<VulkanFramebuffer> swapChainFrameBuffers;
    VulkanRenderPass mainRenderPass;

    VulkanImageBuffer depthBuffer;

    uint32_t currentFrame = 0;
    uint32_t currentSwapChainImage = 0;

    // Dynamic resources -----------------------------------------------------
    std::unique_ptr<VulkanVertexInput> vertex_3P_3C_3N_2U;
    std::unique_ptr<VulkanDescriptorSetLayout> cameraDescriptorLayout;
    std::unique_ptr<VulkanDescriptorSetLayout> materialDescriptorLayout;
    std::unique_ptr<VulkanPipeline> pipeline;
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;

    // ----- Per Frame resources
    std::vector<VulkanDescriptorSet> perFrameDescriptorSets;
    std::vector<VulkanUniformBuffer> perFrameUniformBuffers;
    UniformBufferContent<CameraUbo> uboContent;


    // TEMP
    RenderMesh quadMesh;
};

