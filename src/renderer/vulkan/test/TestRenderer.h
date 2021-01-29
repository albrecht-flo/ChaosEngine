#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan usese 0 to 1

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <memory>
#include <unordered_map>

#include "MainSceneRenderPass.h"
#include "PostRenderPass.h"
#include "ImGuiRenderPass.h"

#include "VulkanRenderer.h"
#include "src/renderer/vulkan/command/VulkanCommandBuffer.h"
#include "src/renderer/vulkan/image/VulkanImageView.h"
#include "src/renderer/vulkan/image/VulkanImage.h"
#include "src/renderer/vulkan/image/VulkanSampler.h"
#include "src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "src/renderer/vulkan/memory/VulkanMemory.h"
#include "src/renderer/vulkan/memory/VulkanBuffer.h"

#include "src/renderer/data/Mesh.h"
#include "src/renderer/data/RenderObject.h"
#include "src/renderer/vulkan/image/VulkanTexture.h"

#define MAX_FRAMES_IN_FLIGHT 2


class TestRenderer : public VulkanRenderer {
public:
    explicit TestRenderer(Window &w);

    ~TestRenderer() = default;

// Virtual methods of super class
public:
    void drawFrame() override;

    void init() override;

protected:
    void destroyResources() override;

public: // RenderObject handling
    RenderMesh uploadMesh(Mesh &mesh);

    RenderObjectRef addObject(RenderMesh &rmesh, glm::mat4 modelMat, MaterialRef material);

    bool setModelMatrix(uint32_t robjID, glm::mat4 modelMat);

    MaterialRef createMaterial(const TexturePhongMaterial& material);

    // Camera handling
    void setViewMatrix(const glm::mat4 &view);

    void setCameraAngle(float angle);

    glm::mat4 getViewMatrix() const { return camera.view; }

private: // RenderObject data
    std::vector<RenderMesh> meshes;
    std::vector<RenderObject> renderObjects;
    RenderMesh quadMesh{};
    RenderObject quadRobj{nullptr, glm::mat4(), 0};

    Camera camera{
            .view = glm::mat4(),
            .angle = 45.0f,
            .near = 0.1f,
            .far = 100.0f
    };

private:
    void createImageBuffers();

    void createFramebuffers();

    void createCommandPool();

    void createDepthResources();

    void createCommandBuffers();

    void createSyncObjects();

    // per frame
    void updateUniformBuffer(uint32_t currentImage);

    void updateCommandBuffer(uint32_t currentImage);

    // cleanup
    void recreateSwapChain();

    void cleanupSwapChain();

private:
    // The swap chain
    VulkanSwapChain swapChain;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    // Frame resources for offscreen scene rendering
    VkFramebuffer offscreenFramebuffer{};
    VkImage offscreenImage{};
    VkDeviceMemory offscreenImageMemory{};
    VkImageView offscreenImageView{};

    // Frame resources for offscreen ImGui rendering
    VkFramebuffer imGuiFramebuffer{};
    VkImage imGuiImage{};
    VkDeviceMemory imGuiImageMemory{};
    VkImageView imGuiImageView{};

    // The command pool, TOBE moved
    VulkanCommandPool commandPool;

    // The memory management object
    VulkanMemory vulkanMemory;

    // Render passes
    MainSceneRenderPass mainGraphicsPass;
    ImGuiRenderPass imGuiRenderPass;
    PostRenderPass postRenderPass;

    // Depth resources
    VkImage depthImage{};
    VkDeviceMemory depthImageMemory{};
    VkImageView depthImageView{};

    // The buffers containing the queue commands
    std::vector<VulkanCommandBuffer> commandBuffers{};

    // The sync objects
    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VkFence> inFlightFences{};

    // Counter
    size_t currentFrame = 0;
};

