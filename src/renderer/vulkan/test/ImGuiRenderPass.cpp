#include "ImGuiRenderPass.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "src/renderer/window/Window.h"
#include "src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"

#include <stdexcept>


static void check_imgui_vk_result(VkResult result) {
    if (result != VK_SUCCESS)
        throw std::runtime_error("[ERROR] [ImGui - Vulkan] Error in ImGui Vulkan code.");

}

/* Configures the render rendering with the attachments and subpasses */
ImGuiRenderPass::ImGuiRenderPass(VulkanDevice &device,
                                 VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain, Window &window,
                                 const VulkanInstance &instance) :
        VulkanRenderPassOld(device, vulkanMemory, swapChain), window(window), instance(instance) {
}

void ImGuiRenderPass::init() {
    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(device, AttachmentType::Color).build());
    renderPass = std::make_unique<VulkanRenderPass>(VulkanRenderPass::Create(device, attachments));

    // Create descriptor pool for ImGui
    descriptorPool = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(device)
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
                    .setMaxSets(1000 * 11)
                    .build()
    );

    // Initialize ImGui

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    // Init imgui window
    ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

    // Setup vulkan for ImGui
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance.vk();
    init_info.PhysicalDevice = device.getPhysicalDevice();
    init_info.Device = device.vk();
    init_info.QueueFamily = device.getPresentQueueFamily();
    init_info.Queue = device.getPresentQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool->vk();
    init_info.Allocator = nullptr;
    init_info.MinImageCount = swapChain.size();
    init_info.ImageCount = swapChain.size();
    init_info.CheckVkResultFn = check_imgui_vk_result;
    ImGui_ImplVulkan_Init(&init_info, renderPass->vk());

    // Upload font texture
    VkCommandBuffer cmdBuf = vulkanMemory.beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(cmdBuf);
    vulkanMemory.endSingleTimeCommands(cmdBuf);
}

// Rendering stuff
/* Begin the render rendering and setup all context descriptors . */
void ImGuiRenderPass::cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer,
                               uint32_t viewportWidth, uint32_t viewportHeight) {
    // Define render rendering to draw with
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->vk(); // the renderpass to use
    renderPassInfo.framebuffer = framebuffer; // the attatchment
    renderPassInfo.renderArea.offset = {0, 0}; // size of the render area ...
    renderPassInfo.renderArea.extent = swapChain.getExtent(); // based on swap chain

    // Define the values used for VK_ATTACHMENT_LOAD_OP_CLEAR
    std::array<VkClearValue, 1> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // use commands in primary cmdbuffer

    // Record ImGui commands to command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf);
}

/* Setup all descriptors and push commands for this render object. */
void ImGuiRenderPass::cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) {

}

/* End this render rendering. */
void ImGuiRenderPass::cmdEnd(VkCommandBuffer &cmdBuf) {
    vkCmdEndRenderPass(cmdBuf);
}

void ImGuiRenderPass::destroySwapChainDependent() {

}

/* Recreates this render rendering to fit the new swap chain. */
void ImGuiRenderPass::recreate() {
    ImGui_ImplVulkan_SetMinImageCount(swapChain.size());
}

void ImGuiRenderPass::destroy() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}