#include "ImGuiRenderPass.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <stdexcept>

#include "src/renderer/window/Window.h"

static void check_imgui_vk_result(VkResult result) {
    if (result != VK_SUCCESS)
        throw std::runtime_error("[ImGui - Vulkan] Error in ImGui Vulkan code.");

}

/* Configures the render pass with the attachments and subpasses */
ImGuiRenderPass::ImGuiRenderPass(VulkanDevice &device,
                                 VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain, Window &window, const VulkanInstance &instance) :
        VulkanRenderPass(device, vulkanMemory, swapChain), window(window), instance(instance) {
}

void ImGuiRenderPass::init() {
    createRenderPass();

    // Create descriptor pool for ImGui
    descriptorPool = VulkanDescriptor::createPool(device,
                                                  {
                                                          VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                                          VkDescriptorPoolSize{
                                                                  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                                          VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                                               1000},
                                                          VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                                               1000},
                                                          VkDescriptorPoolSize{
                                                                  VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                                          VkDescriptorPoolSize{
                                                                  VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                                          VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                               1000},
                                                          VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                                               1000},
                                                          VkDescriptorPoolSize{
                                                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                                          VkDescriptorPoolSize{
                                                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                                          VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                                                               1000}
                                                  });

    // Initialize ImGui

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
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
    init_info.DescriptorPool = descriptorPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = swapChain.size();
    init_info.ImageCount = swapChain.size();
    init_info.CheckVkResultFn = check_imgui_vk_result;
    ImGui_ImplVulkan_Init(&init_info, renderPass);

    // Upload font texture
    VkCommandBuffer cmdBuf = vulkanMemory.beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(cmdBuf);
    vulkanMemory.endSingleTimeCommands(cmdBuf);
}

/* Creates the vulkan render pass, describing all attachments, subpasses and subpass dependencies. */
void ImGuiRenderPass::createRenderPass() {
    // Configures color attachment processing
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling so only 1
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear before new frame
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store results instead of discarding them
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // layout ~before~ render pass
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // layout ~after~ render pass

    // Subpasses references one or more color attachments
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0; // reference to the attachments array passed to the subpass
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // color buffer attachment

    // For the moment we only have 1 subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // graphics not compute
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Configure subpass dependency
    // We want our subpass to wait for the previous stage to finish reading the color attachment
    std::array<VkSubpassDependency, 2> dependencies = {};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // implicit prior subpass
    dependencies[0].dstSubpass = 0; // ! must be higher than srcSubpass, VK_SUBPASS_EXTERNAL would be implicit next subpass
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // want to wait for swap chain to finish reading framebuffer
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // make following color subpasses wait for this one to finish
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    dependencies[1].srcSubpass = 0; // implicit prior subpass
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL; // ! must be higher than srcSubpass, VK_SUBPASS_EXTERNAL would be implicit next subpass
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // want to wait for swap chain to finish reading framebuffer
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // make following color subpasses wait for this one to finish
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Combine subpasses, dependencies and attachments to render pass
    std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device.vk(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create ImGui render pass!");
    }

#ifdef M_DEBUG
    std::cout << "ImGuiRenderPass: created render pass (" << renderPass << ")" << std::endl;
#endif
}

// Rendering stuff
/* Begin the render pass and setup all context descriptors . */
void ImGuiRenderPass::cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) {
    // Define render pass to draw with
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass; // the renderpass to use
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

/* End this render pass. */
void ImGuiRenderPass::cmdEnd(VkCommandBuffer &cmdBuf) {
    vkCmdEndRenderPass(cmdBuf);
}

void ImGuiRenderPass::destroySwapChainDependent() {

}

/* Recreates this render pass to fit the new swap chain. */
void ImGuiRenderPass::recreate() {
    ImGui_ImplVulkan_SetMinImageCount(swapChain.size());
}

void ImGuiRenderPass::destroy() {
    // The descriptor pool and sets depend also on the number of images in the swapchain
    vkDestroyDescriptorPool(device.vk(), descriptorPool,
                            nullptr); // this also destroys the descriptor sets of this pools

    vkDestroyRenderPass(device.vk(), renderPass, nullptr);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}