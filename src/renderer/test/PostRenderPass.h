#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <array>

#include "../VulkanRenderPass.h"
#include "../general/VulkanDevice.h"
#include "../general/VulkanSwapChain.h"
#include "../image/VulkanImage.h"
#include "../image/VulkanSampler.h"
#include "../pipeline/VulkanPipeline.h"
#include "../pipeline/VulkanDescriptor.h"
#include "../data/Mesh.h"
#include "../data/VulkanTexture.h"
#include "../data/RenderObject.h"

class PostRenderPass : public VulkanRenderPass {
public:
    PostRenderPass(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain);

    ~PostRenderPass() = default;

    void
    setImageBufferViews(VkImageView newFramebufferView, VkImageView newDepthBufferView, VkImageView imGuiImageView);

    void updateCamera(const Camera &newCamera) { camera = newCamera; }

    void init() override;

    void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) override;

    void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) override;

    void cmdEnd(VkCommandBuffer &cmdBuf) override;

    void recreate() override;

    void destroy() override;

    void destroySwapChainDependent() override;

private:
    void createRenderPass();

    void createPipelineAndDescriptors();

private:
    // Pipelines
    DescriptorSetLayout descriptorSetLayout{};
    PipelineLayout postprocessingPipelineLayout{};
    VulkanPipeline postprocessingPipeline{};

    VkImageView framebufferView{};
    VkSampler framebufferSampler{};
    VkImageView depthBufferView{};
    VkSampler depthBufferSampler{};
    VkImageView imGuiImageView{};
    VkSampler imGuiImageSampler{};
    // The objects for uniform buffer linking
    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};

    Camera camera{};
    VulkanTexture backgroundTexture{};
};

