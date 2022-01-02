#pragma once

#include "Engine/src/renderer/window/Window.h"
#include "Engine/src/renderer/vulkan/context/VulkanContext.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPool.h"

#include <imgui.h>

class ImGuiRenderingPass {
private:
    ImGuiRenderingPass(const VulkanContext &context, std::unique_ptr<VulkanRenderPass> &&renderPass,
                       std::vector<VulkanFramebuffer> &&swapChainFrameBuffers,
                       std::unique_ptr<VulkanDescriptorPool> &&descriptorPool,
                       ImGuiContext *imGuiContext);

    void destroy();

public:
    ~ImGuiRenderingPass() { destroy(); }

    ImGuiRenderingPass(const ImGuiRenderingPass &o) = delete;

    ImGuiRenderingPass &operator=(const ImGuiRenderingPass &o) = delete;

    ImGuiRenderingPass(ImGuiRenderingPass &&o) noexcept;

    ImGuiRenderingPass &operator=(ImGuiRenderingPass &&o) = delete;


    static ImGuiRenderingPass Create(const VulkanContext &context, const Window &window);

    void draw();

    void resizeAttachments(uint32_t width, uint32_t height);

private:
    const VulkanContext &context;
    std::unique_ptr<VulkanRenderPass> renderPass;
    std::vector<VulkanFramebuffer> swapChainFrameBuffers;
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;
    ImGuiContext *imGuiContext;
};
