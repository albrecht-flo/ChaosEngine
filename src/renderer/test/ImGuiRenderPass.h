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

class ImGuiRenderPass : public VulkanRenderPass {
public:
    ImGuiRenderPass(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain, Window &window);

    ~ImGuiRenderPass() = default;

    void init() override;

    void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) override;

    void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) override;

    void cmdEnd(VkCommandBuffer &cmdBuf) override;

    void recreate() override;

    void destroy() override;

    void destroySwapChainDependent() override;

private:
    void createRenderPass();

private:
    Window &window;
    // The objects for uniform buffer linking
    VkDescriptorPool descriptorPool = {};
};

