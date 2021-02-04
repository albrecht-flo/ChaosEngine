#include "PostRenderPass.h"

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h>
#include <src/renderer/vulkan/rendering/VulkanRenderPass.h>

/* Configures the render rendering with the attachments and subpasses */
PostRenderPass::PostRenderPass(VulkanDevice &device,
                               VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain) :
        VulkanRenderPassOld(device, vulkanMemory, swapChain), backgroundTexture(device) {
}

void PostRenderPass::init() {
    // Create the render rendering
    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(device, AttachmentType::Color).build());
    renderPass = std::make_unique<VulkanRenderPass>(VulkanRenderPass::Create(device, attachments));


    // This descriptor set contains the textures for composition
    descriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetBuilder(device)
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
    auto attributeDescription = Vertex::getAttributeDescriptions();
    postprocessingPipeline = std::make_unique<VulkanPipeline>(
            VulkanPipeline::Create(device,
                                   Vertex::getBindingDescription(),
                                   attributeDescription.data(),
                                   static_cast<uint32_t>(attributeDescription.size()),
                                   swapChain.getExtent(), std::move(postprocessingPipelineLayout),
                                   renderPass->vk(),
                                   "post", false
            ));



    // Create descriptor pool
    descriptorPool = VulkanDescriptor::createPool(device,
                                                  {
                                                          VkDescriptorPoolSize{ // Color attachment from main scene rendering
                                                                  .type = descriptorSetLayout->getBinding(
                                                                          0).descriptorType,
                                                                  .descriptorCount = 1
                                                          },
                                                          VkDescriptorPoolSize{ // Depth attachment form main scene rendering
                                                                  .type = descriptorSetLayout->getBinding(
                                                                          1).descriptorType,
                                                                  .descriptorCount = 1
                                                          },
                                                          VkDescriptorPoolSize{ // Background texture
                                                                  .type = descriptorSetLayout->getBinding(
                                                                          2).descriptorType,
                                                                  .descriptorCount = 1
                                                          },
                                                          VkDescriptorPoolSize{ // ImGui framebuffer texture
                                                                  .type = descriptorSetLayout->getBinding(
                                                                          3).descriptorType,
                                                                  .descriptorCount = 1
                                                          },
                                                  });

    descriptorSet = VulkanDescriptor::allocateDescriptorSet(device,
                                                            *descriptorSetLayout, descriptorPool);

    // Create the samplers for the attachments of previous passes
    framebufferSampler = VulkanSampler::create(device);
    depthBufferSampler = VulkanSampler::create(device);
    imGuiImageSampler = VulkanSampler::create(device);

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
    VulkanDescriptor::writeDescriptorSet(device, descriptorSet,
                                         {}, // No buffers
                                         {
                                                 DescriptorImageInfo{
                                                         .descriptorInfo = VkDescriptorImageInfo{
                                                                 .sampler =   framebufferSampler,
                                                                 .imageView = framebufferView,
                                                                 .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                         },
                                                         .binding = 0,
                                                         .arrayElement = 0,
                                                         .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                         .count = 1
                                                 },
                                                 DescriptorImageInfo{
                                                         .descriptorInfo = VkDescriptorImageInfo{
                                                                 .sampler =   depthBufferSampler,
                                                                 .imageView = depthBufferView,
                                                                 .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                                         },
                                                         .binding = 1,
                                                         .arrayElement = 0,
                                                         .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                         .count = 1
                                                 },
                                                 DescriptorImageInfo{
                                                         .descriptorInfo = VkDescriptorImageInfo{
                                                                 .sampler =   backgroundTexture.getSampler(),
                                                                 .imageView = backgroundTexture.getImageView().vk(),
                                                                 .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                         },
                                                         .binding = 2,
                                                         .arrayElement = 0,
                                                         .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                         .count = 1
                                                 },
                                                 DescriptorImageInfo{
                                                         .descriptorInfo = VkDescriptorImageInfo{
                                                                 .sampler =   imGuiImageSampler,
                                                                 .imageView = imGuiImageView,
                                                                 .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                         },
                                                         .binding = 3,
                                                         .arrayElement = 0,
                                                         .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                         .count = 1
                                                 }
                                         }
    );
}

// Rendering stuff
/* Begin the render rendering and setup all context descriptors . */
void PostRenderPass::cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) {

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

    // Bind the descriptor set to the pipeline
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            postprocessingPipeline->getPipelineLayout(),
                            0, 1, &descriptorSet, 0, nullptr);

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
    // Pipeline creation
    auto attributeDescription = Vertex::getAttributeDescriptions();
    *postprocessingPipeline = VulkanPipeline::Create(device,
                                                     Vertex::getBindingDescription(),
                                                     attributeDescription.data(),
                                                     static_cast<uint32_t>(attributeDescription.size()),
                                                     swapChain.getExtent(),
                                                     postprocessingPipeline->releasePipelineLayout(),
                                                     renderPass->vk(),
                                                     "post", false
    );
}

void PostRenderPass::destroy() {

    // These uniform buffers are per frame and therefore depend on the number of swapchain images
    VulkanSampler::destroy(device, framebufferSampler);
    VulkanSampler::destroy(device, depthBufferSampler);
    VulkanSampler::destroy(device, imGuiImageSampler);

    // The descriptor pool and sets depend also on the number of images in the swapchain
    vkDestroyDescriptorPool(device.vk(), descriptorPool,
                            nullptr); // this also destroys the descriptor sets of this pools
}