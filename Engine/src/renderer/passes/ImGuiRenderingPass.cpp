#include "ImGuiRenderingPass.h"

#include "Engine/src/renderer/vulkan/image/VulkanImageView.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPoolBuilder.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <array>

static void check_imgui_vk_result(VkResult result) {
    if (result != VK_SUCCESS)
        throw std::runtime_error("[ERROR] [ImGui - Vulkan] Error in ImGui Vulkan code.");

}

static std::vector<VulkanFramebuffer>
createSwapChainFrameBuffers(const VulkanDevice &device, const VulkanSwapChain &swapChain,
                            const VulkanRenderPass &renderPass) {
    std::vector<VulkanFramebuffer> swapChainFramebuffers;
    swapChainFramebuffers.reserve(swapChain.size());
    for (uint32_t i = 0; i < swapChain.size(); i++) {
        swapChainFramebuffers.emplace_back(
                renderPass.createFrameBuffer(
                        {swapChain.getImageViews()[i].vk()},
                        swapChain.getExtent()
                ));
    }
    return std::move(swapChainFramebuffers);
}

// ------------------------------------ Class Members ------------------------------------------------------------------

ImGuiRenderingPass
ImGuiRenderingPass::Create(const VulkanContext &context, const Window &window, uint32_t width, uint32_t height,
                           ImGuiContext *imGuiContext) {

    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(context.getDevice(), AttachmentType::Color)
                                     .loadStore(AttachmentLoadOp::Preserve, AttachmentStoreOp::Store)
                                     .layoutInitFinal(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                                     .build());
    auto renderPass = std::make_unique<VulkanRenderPass>(VulkanRenderPass::Create(context.getDevice(), attachments));

    auto swapChainFrameBuffers = createSwapChainFrameBuffers(context.getDevice(), context.getSwapChain(), *renderPass);

    // Create descriptor pool for ImGui
    auto descriptorPool = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(context.getDevice())
                    .addDescriptor(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
                    .addDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                    .addDescriptor(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
                    .addDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
                    .addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
                    .addDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
                    .addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                    .addDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
                    .addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
                    .addDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
                    .addDescriptor(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
                    .setMaxSets(1000 * 5)
                    .build());

    // Init imgui window
    ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

    // Setup vulkan for ImGui
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = context.getInstance().vk();
    init_info.PhysicalDevice = context.getDevice().getPhysicalDevice();
    init_info.Device = context.getDevice().vk();
    init_info.QueueFamily = context.getDevice().getPresentQueueFamily();
    init_info.Queue = context.getDevice().getPresentQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool->vk();
    init_info.Allocator = nullptr;
    init_info.MinImageCount = context.getSwapChain().size();
    init_info.ImageCount = context.getSwapChain().size();
    init_info.CheckVkResultFn = check_imgui_vk_result;
    ImGui_ImplVulkan_Init(&init_info, renderPass->vk());

    // Upload font texture
    VkCommandBuffer cmdBuf = context.getMemory().beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(cmdBuf);
    context.getMemory().endSingleTimeCommands(cmdBuf);


    return ImGuiRenderingPass(context, std::move(renderPass), std::move(swapChainFrameBuffers),
                              std::move(descriptorPool), imGuiContext);
}

ImGuiRenderingPass::ImGuiRenderingPass(const VulkanContext &context, std::unique_ptr<VulkanRenderPass> &&renderPass,
                                       std::vector<VulkanFramebuffer> &&swapChainFrameBuffers,
                                       std::unique_ptr<VulkanDescriptorPool> &&descriptorPool,
                                       ImGuiContext *imGuiContext)
        : context(context), renderPass(std::move(renderPass)), swapChainFrameBuffers(std::move(swapChainFrameBuffers)),
          descriptorPool(std::move(descriptorPool)), imGuiContext(imGuiContext) {}


ImGuiRenderingPass::ImGuiRenderingPass(ImGuiRenderingPass &&o) noexcept
        : context(o.context), renderPass(std::move(o.renderPass)),
          swapChainFrameBuffers(std::move(o.swapChainFrameBuffers)), descriptorPool(std::move(o.descriptorPool)),
          imGuiContext(std::exchange(o.imGuiContext, nullptr)) {}

void ImGuiRenderingPass::draw() {
    auto &cmdBuf = context.getCurrentPrimaryCommandBuffer();
    auto &framebuffer = swapChainFrameBuffers[context.getCurrentSwapChainFrame()];
// Define render rendering to draw with
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->vk(); // the renderpass to use
    renderPassInfo.framebuffer = framebuffer.vk(); // the attatchment
    renderPassInfo.renderArea.offset = {0, 0}; // size of the render area ...
    renderPassInfo.renderArea.extent = context.getSwapChain().getExtent(); // based on swap chain
    renderPassInfo.clearValueCount = 0;

    vkCmdBeginRenderPass(cmdBuf.vk(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // use commands in primary cmdbuffer

    // Record ImGui commands to command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf.vk());

    vkCmdEndRenderPass(cmdBuf.vk());
}

void ImGuiRenderingPass::resizeAttachments(uint32_t, uint32_t) {
    swapChainFrameBuffers = createSwapChainFrameBuffers(context.getDevice(), context.getSwapChain(), *renderPass);

    ImGui_ImplVulkan_SetMinImageCount(context.getSwapChain().size());
}

void ImGuiRenderingPass::destroy() {
    if (imGuiContext != nullptr) {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}
