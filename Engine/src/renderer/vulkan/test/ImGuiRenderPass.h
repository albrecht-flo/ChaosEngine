#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <array>

#include "VulkanRenderPassOld.h"
#include "Engine/src/renderer/vulkan/context/VulkanInstance.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPool.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"

// Base on ImGui Vulkan examples and https://frguthmann.github.io/posts/vulkan_imgui/

struct RenderObject;
class ImGuiRenderPass : public VulkanRenderPassOld {
public:
    ImGuiRenderPass(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain, Window &window,
                       const VulkanInstance &instance);

    ~ImGuiRenderPass() = default;

    void init() override;

    void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer, uint32_t viewportWidth = 0,
                  uint32_t viewportHeight = 0) override;

    void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) override;

    void cmdEnd(VkCommandBuffer &cmdBuf) override;

    void recreate() override;

    void destroy() override;

    void destroySwapChainDependent() override;

    [[nodiscard]] inline VkRenderPass vk() const { return renderPass->vk(); }

private:
    Window &window;
    const VulkanInstance &instance;
    // The objects for uniform buffer linking
    std::unique_ptr<VulkanRenderPass> renderPass;
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;
};

