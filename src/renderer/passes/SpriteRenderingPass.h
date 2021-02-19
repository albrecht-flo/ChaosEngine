#pragma once

class RenderingPass {
public:

};

#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan usese 0 to 1
#include <glm/glm.hpp>
#include "src/renderer/vulkan/context/VulkanContext.h"
#include <src/renderer/vulkan/pipeline/VulkanVertexInput.h>
#include <src/renderer/vulkan/image/VulkanImage.h>
#include <src/renderer/data/RenderObject.h>
#include <src/renderer/vulkan/pipeline/VulkanVertexInput.h>
#include "src/renderer/vulkan/rendering/VulkanFrame.h"
#include "src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "src/renderer/vulkan/context/VulkanContext.h"
#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorPool.h"

#include <string>
#include <src/core/Components.h>

// ------------------------------------ Pipeline Description -----------------------------------------------------------

class SpriteRenderingPass {
private:
    /// Camera uniform object. Alignas 16 to ensure proper alignment with the GPU storage
    struct CameraUbo {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };
private:
    explicit SpriteRenderingPass(const VulkanContext &context, bool renderToSwapChain = false)
            : context(context), renderToSwapChain(renderToSwapChain) {}

    void init(uint32_t width, uint32_t height);

public:
    ~SpriteRenderingPass() = default;

    SpriteRenderingPass(const SpriteRenderingPass &o) = delete;

    SpriteRenderingPass &operator=(const SpriteRenderingPass &o) = delete;

    SpriteRenderingPass(SpriteRenderingPass &&o) noexcept;

    SpriteRenderingPass &operator=(SpriteRenderingPass &&o) = delete;

    static SpriteRenderingPass
    Create(const VulkanContext &context, uint32_t width, uint32_t height, bool renderToSwapChain = false);

    void begin(const CameraComponent &camera);

    void end();

    void resizeAttachments(uint32_t width, uint32_t height);

    void drawSprite(const RenderMesh &renderObject, const glm::mat4 &modelMat, const glm::vec4 &color);

    inline const VulkanImageBuffer &getColorBuffer() { return *colorBuffer; }

    inline const VulkanImageBuffer &getDepthBuffer() { return *depthBuffer; }

private:
    void createAttachments(uint32_t width, uint32_t height);

    void updateUniformBuffer(const CameraComponent &camera, const glm::vec2 &viewportDimensions);

    void createStandardPipeline();

private:
    const VulkanContext &context;
    std::unique_ptr<VulkanRenderPass> opaquePass;
    bool renderToSwapChain;

    std::unique_ptr<VulkanImageBuffer> colorBuffer;
    std::unique_ptr<VulkanImageBuffer> depthBuffer;
    std::unique_ptr<VulkanFramebuffer> framebuffer;
    std::vector<VulkanFramebuffer> swapChainFrameBuffers;

    // Dynamic resources ------------------------------------------------------
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;
    std::unique_ptr<VulkanDescriptorSetLayout> cameraDescriptorLayout;
    std::unique_ptr<VulkanDescriptorSetLayout> materialDescriptorLayout;
    std::unique_ptr<VulkanPipeline> pipeline;

    // Per Frame resources ----------------------------------------------------
    std::vector<VulkanDescriptorSet> perFrameDescriptorSets;
    std::vector<VulkanUniformBuffer> perFrameUniformBuffers;
    UniformBufferContent<CameraUbo> uboContent;
};

