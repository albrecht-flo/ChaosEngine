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

#include "../VulkanRenderer.h"
#include "../general/VulkanCommandBuffer.h"
#include "../image/VulkanImageView.h"
#include "../image/VulkanImage.h"
#include "../image/VulkanSampler.h"
#include "../image/VulkanFramebuffer.h"
#include "../memory/VulkanMemory.h"
#include "../memory/VulkanBuffer.h"

#include "../data/Mesh.h"
#include "../data/RenderObject.h"
#include "../data/VulkanTexture.h"

#define MAX_FRAMES_IN_FLIGHT 2


class TestRenderer : public VulkanRenderer {
public:
    TestRenderer(Window &w);

    ~TestRenderer();
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

    MaterialRef createMaterial(const TexturePhongMaterial material);

    // Camera handling
    void setViewMatrix(const glm::mat4 &view);

    void setCameraAngle(float angle);

    glm::mat4 getViewMatrix() const { return m_camera.view; }

private: // RenderObject data
    std::vector<RenderMesh> m_meshes;
    std::vector<RenderObject> m_renderObjects;
    RenderMesh m_quadMesh;
    RenderObject m_quadRobj{nullptr, glm::mat4(), 0};

    Camera m_camera{
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
    VulkanSwapChain m_swapChain;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    // Frame ressources for offscreen scene rendering
    VkFramebuffer m_offscreenFramebuffer;
    VkImage m_offscreenImage;
    VkDeviceMemory m_offscreenImageMemory;
    VkImageView m_offscreenImageView;

    // Frame ressources for offscreen ImGui renering
    VkFramebuffer m_imGuiFramebuffer;
    VkImage m_imGuiImage;
    VkDeviceMemory m_imGuiImageMemory;
    VkImageView m_imGuiImageView;

    // The command pool, TOBE moved
    VkCommandPool m_commandPool = {};

    // Render passes
    MainSceneRenderPass m_mainGraphicsPass;
    PostRenderPass m_postRenderPass;
    ImGuiRenderPass m_imGuiRenderPass;

    // The memeory management object
    std::unique_ptr<VulkanMemory> m_vulkanMemory;

    // Depth resources
    VkImage m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkImageView m_depthImageView;

    // The buffers containing the queue commands
    std::vector<VulkanCommandBuffer> m_commandBuffers;

    // The sync objects
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    // Counter
    size_t m_currentFrame = 0;
};

