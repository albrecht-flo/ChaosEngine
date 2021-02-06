#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <array>

#include "VulkanRenderPassOld.h"
#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "src/renderer/vulkan/context/VulkanSwapChain.h"
#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "src/renderer/vulkan/image/VulkanImage.h"
#include "src/renderer/vulkan/image/VulkanSampler.h"
#include "src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptor.h"
#include "src/renderer/data/Mesh.h"
#include "src/renderer/vulkan/image/VulkanTexture.h"
#include "src/renderer/data/RenderObject.h"

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
    VkDescriptorPool descriptorPool = {};
    std::unique_ptr<VulkanRenderPass> renderPass;
};

