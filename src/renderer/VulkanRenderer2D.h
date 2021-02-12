#pragma once


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan usese 0 to 1

#include <glm/glm.hpp>

#include <src/renderer/vulkan/image/VulkanImage.h>
#include <src/renderer/data/RenderObject.h>
#include <src/renderer/vulkan/pipeline/VulkanVertexInput.h>
#include <src/renderer/passes/SpriteRenderingPass.h>
#include "src/renderer/vulkan/rendering/VulkanFrame.h"
#include "src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "src/renderer/vulkan/context/VulkanContext.h"
#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "src/renderer/vulkan/pipeline/VulkanPipeline.h"

class VulkanRenderer2D {
private:
private:
    VulkanRenderer2D(std::unique_ptr<VulkanContext> &&context, SpriteRenderingPass &&spriteRenderingPass);

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

private:
    std::unique_ptr<VulkanContext> context;

    SpriteRenderingPass spriteRenderingPass;
//    PostProcessingPass postProcessingPass;
//    ImGuiRenderingPass imGuiPass;

    // TEMP
    RenderMesh quadMesh;
};

