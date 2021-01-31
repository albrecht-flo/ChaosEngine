#include "PostRenderPass.h"

#include <iostream>
#include <stdexcept>
#include <cstring>

/* Configures the render rendering with the attachments and subpasses */
PostRenderPass::PostRenderPass(VulkanDevice &device,
                               VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain) :
        VulkanRenderPass(device, vulkanMemory, swapChain),
        backgroundTexture(device) {
}

void PostRenderPass::init() {
    // Create the render rendering
    createRenderPass();

    // This descriptor set contains the textures for composition
    descriptorSetLayout = VulkanDescriptor::createDescriptorSetLayout(
            device,
            {
                    VkDescriptorSetLayoutBinding{ // Color attachment from main scene
                            .binding=0,
                            .descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .descriptorCount=1,
                            .stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT,
                            .pImmutableSamplers=nullptr
                    },
                    VkDescriptorSetLayoutBinding{ // Depth attachment from main scene
                            .binding=1,
                            .descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .descriptorCount=1,
                            .stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT,
                            .pImmutableSamplers=nullptr
                    },
                    VkDescriptorSetLayoutBinding{ // Background texture
                            .binding=2,
                            .descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .descriptorCount=1,
                            .stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT,
                            .pImmutableSamplers=nullptr
                    },
                    VkDescriptorSetLayoutBinding{ // ImGui framebuffer texture
                            .binding=3,
                            .descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .descriptorCount=1,
                            .stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT,
                            .pImmutableSamplers=nullptr
                    },
            });


    // Pipeline layout
    postprocessingPipelineLayout = PipelineLayout{
            .layouts = {
                    descriptorSetLayout
            },
            .pushConstants = {
                    VkPushConstantRange{
                            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                            .offset = 0,
                            .size = sizeof(float) * 2
                    }
            }
    };


    // Create descriptor pool
    descriptorPool = VulkanDescriptor::createPool(device,
                                                  {
                                                          VkDescriptorPoolSize{ // Color attachment from main scene rendering
                                                                  .type = descriptorSetLayout.bindings[0].descriptorType,
                                                                  .descriptorCount = 1
                                                          },
                                                          VkDescriptorPoolSize{ // Depth attachment form main scene rendering
                                                                  .type = descriptorSetLayout.bindings[1].descriptorType,
                                                                  .descriptorCount = 1
                                                          },
                                                          VkDescriptorPoolSize{ // Background texture
                                                                  .type = descriptorSetLayout.bindings[2].descriptorType,
                                                                  .descriptorCount = 1
                                                          },
                                                          VkDescriptorPoolSize{ // ImGui framebuffer texture
                                                                  .type = descriptorSetLayout.bindings[3].descriptorType,
                                                                  .descriptorCount = 1
                                                          },
                                                  });

    descriptorSet = VulkanDescriptor::allocateDescriptorSet(device,
                                                            descriptorSetLayout, descriptorPool);

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

/* Creates the vulkan render rendering, describing all attachments, subpasses and subpass dependencies. */
void PostRenderPass::createRenderPass() {
    // Configures color attachment processing
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChain.getFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling so only 1
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear before new frame
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store results instead of discarding them
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // should be presented in the swap chain

    // Subpasses references one or more color attachments
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0; // reference to the attachments array passed to the subpass
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // color buffer attachment

    // For the moment we only have 1 subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // graphics not compute
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = nullptr;

    // Configure subpass dependency
    // We want our subpass to wait for the previous stage to finish reading the color attachment
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // implicit prior subpass
    dependency.dstSubpass = 0; // ! must be higher than srcSubpass, VK_SUBPASS_EXTERNAL would be implicit next subpass
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // want to wait for swap chain to finish reading framebuffer
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // make following color subpasses wait for this one to finish
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


    // Combine subpasses, dependencies and attachments to render rendering
    std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device.vk(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create render rendering!");
    }

#ifdef M_DEBUG
    std::cout << "PostRenderPass: created render rendering (" << renderPass << ")" << std::endl;
#endif
}

void PostRenderPass::createPipelineAndDescriptors() {
    // Pipeline creation
    auto attributeDescription = Vertex::getAttributeDescriptions();
    postprocessingPipeline = VulkanPipeline::create(device,
                                                    Vertex::getBindingDescription(),
                                                    attributeDescription.data(),
                                                    static_cast<uint32_t>(attributeDescription.size()),
                                                    swapChain.getExtent(), postprocessingPipelineLayout,
                                                    renderPass,
                                                    "post", false
    );

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
    renderPassInfo.renderPass = renderPass; // the renderpass to use
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
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, postprocessingPipeline.pipeline);

    // Bind the descriptor set to the pipeline
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            postprocessingPipeline.pipelineLayout,
                            0, 1, &descriptorSet, 0, nullptr);

    vkCmdPushConstants(cmdBuf, postprocessingPipeline.pipelineLayout,
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
    postprocessingPipeline.destroy(device);

    vkDestroyRenderPass(device.vk(), renderPass, nullptr);

}

/* Recreates this render rendering to fit the new swap chain. */
void PostRenderPass::recreate() {
    // Recreate the render rendering, because swap chain format has changed
    createRenderPass();
}

void PostRenderPass::destroy() {

    // These uniform buffers are per frame and therefore depend on the number of swapchain images
    VulkanSampler::destroy(device, framebufferSampler);
    VulkanSampler::destroy(device, depthBufferSampler);
    VulkanSampler::destroy(device, imGuiImageSampler);

    // The descriptor pool and sets depend also on the number of images in the swapchain
    vkDestroyDescriptorPool(device.vk(), descriptorPool,
                            nullptr); // this also destroys the descriptor sets of this pools

    vkDestroyDescriptorSetLayout(device.vk(), descriptorSetLayout.vDescriptorSetLayout, nullptr);
}