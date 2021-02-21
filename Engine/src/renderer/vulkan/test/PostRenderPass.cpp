#include "PostRenderPass.h"

#include "Engine/src/renderer/data/Mesh.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSetLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPoolBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "Engine/src/renderer/RendererAPI.h"

#include <stdexcept>

/* Configures the render rendering with the attachments and subpasses */
PostRenderPass::PostRenderPass(VulkanDevice &device,
                               VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain) :
        VulkanRenderPassOld(device, vulkanMemory, swapChain), backgroundTexture(device),
        // Create the samplers for the attachments of previous passes
        framebufferSampler(std::move(VulkanSampler::create(device))),
        depthBufferSampler(std::move(VulkanSampler::create(device))),
        imGuiImageSampler(std::move(VulkanSampler::create(device))) {
    vertex_3P_3C_3N_2U = VertexAttributeBuilder(0, sizeof(Vertex), InputRate::Vertex)
            .addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            .addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
            .addAttribute(2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
            .addAttribute(3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)).build();
}

void PostRenderPass::init() {
    // Create the render rendering
    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(device, AttachmentType::Color).build());
    renderPass = std::make_unique<VulkanRenderPass>(VulkanRenderPass::Create(device, attachments));


    // This descriptor set contains the textures for composition
    descriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(device)
                    .addBinding(0, DescriptorType::Texture, ShaderStage::Fragment)// Color attachment from main scene
                    .addBinding(1, DescriptorType::Texture, ShaderStage::Fragment)// Depth attachment from main scene
                    .addBinding(2, DescriptorType::Texture, ShaderStage::Fragment)// Background texture
                    .addBinding(3, DescriptorType::Texture, ShaderStage::Fragment)// ImGui framebuffer texture
                    .build()
    );



    // Pipeline layout
    auto postprocessingPipelineLayout = VulkanPipelineLayoutBuilder(device)
            .addPushConstant(sizeof(float) * 2, 0, ShaderStage::Fragment)
            .addDescriptorSet(*descriptorSetLayout)
            .build();

    // Pipeline creation
    postprocessingPipeline = std::make_unique<VulkanPipeline>(
            VulkanPipelineBuilder(device, *renderPass, std::move(postprocessingPipelineLayout), vertex_3P_3C_3N_2U,
                                  "post")
                    .setTopology(Renderer::Topology::TriangleList)
                    .setPolygonMode(Renderer::PolygonMode::Fill)
                    .setCullFace(Renderer::CullFace::CCLW)
                    .setDepthTestEnabled(false)
                    .setDepthCompare(Renderer::CompareOp::Less)
                    .build()
    );



    // Create descriptor pool
    descriptorPool = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(device)
                    .addDescriptor(descriptorSetLayout->getBinding(0).descriptorType,
                                   1)// Color attachment from main scene rendering
                    .addDescriptor(descriptorSetLayout->getBinding(1).descriptorType,
                                   1)// Depth attachment form main scene rendering
                    .addDescriptor(descriptorSetLayout->getBinding(2).descriptorType, 1)// Background texture
                    .addDescriptor(descriptorSetLayout->getBinding(3).descriptorType, 1)// ImGui framebuffer texture
                    .setMaxSets(1)
                    .build()
    );

    descriptorSet = std::make_unique<VulkanDescriptorSet>(descriptorPool->allocate(*descriptorSetLayout));


    // TODO: Remove when reimplementation of 3D is finished
    backgroundTexture = VulkanTexture::createTexture(device, vulkanMemory, "textures/sky.jpg");
}

void PostRenderPass::setImageBufferViews(VkImageView newFramebufferView,
                                         VkImageView newDepthBufferView, VkImageView newImGuiImageView) {
    framebufferView = newFramebufferView;
    depthBufferView = newDepthBufferView;
    imGuiImageView = newImGuiImageView;
    createPipelineAndDescriptors();
}

void PostRenderPass::createPipelineAndDescriptors() {
    // Fill the descriptor set
    descriptorSet->startWriting()
            .writeImageSampler(0, framebufferSampler.vk(), framebufferView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            .writeImageSampler(1, depthBufferSampler.vk(), depthBufferView,
                               VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
            .writeImageSampler(2, backgroundTexture.getSampler(), backgroundTexture.getImageView().vk(),
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            .writeImageSampler(3, imGuiImageSampler.vk(), imGuiImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            .commit();
}

// Rendering stuff
/* Begin the render rendering and setup all context descriptors . */
void PostRenderPass::cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer,
                              uint32_t viewportWidth, uint32_t viewportHeight) {

    // Define render rendering to draw with
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->vk(); // the renderpass to use
    renderPassInfo.framebuffer = framebuffer; // the attatchment
    renderPassInfo.renderArea.offset = {0, 0}; // size of the render area ...
    renderPassInfo.renderArea.extent = swapChain.getExtent(); // based on swap chain

    // Define the values used for VK_ATTACHMENT_LOAD_OP_CLEAR
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // use commands in primary cmdbuffer

    // Now vkCmd... can be written do define the draw call
    // Bind the pipline as a graphics pipeline
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, postprocessingPipeline->getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(viewportWidth);
    viewport.height = static_cast<float>(viewportHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

    VkRect2D scissor = {}; // !!! Important, otherwise ImGui sets this
    scissor.offset = {0, 0};
    scissor.extent = {viewportWidth, viewportHeight};
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    // Bind the descriptor set to the pipeline
    auto desc = descriptorSet->vk();
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            postprocessingPipeline->getPipelineLayout(),
                            0, 1, &desc, 0, nullptr);

    vkCmdPushConstants(cmdBuf, postprocessingPipeline->getPipelineLayout(),
                       VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float) * 2, &camera.near);
}

/* Setup all descriptors and push commands for this render object. */
void PostRenderPass::cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) {
    // TODO
}

/* End this render rendering. */
void PostRenderPass::cmdEnd(VkCommandBuffer &cmdBuf) {
    // Finish the render rendering
    vkCmdEndRenderPass(cmdBuf);
}

void PostRenderPass::destroySwapChainDependent() {
    // The pipeline, layouts and render rendering also deppend on the number of swapchain images and the framebuffers
}

/* Recreates this render rendering to fit the new swap chain. */
void PostRenderPass::recreate() {
}

void PostRenderPass::destroy() {
}