#include "MainSceneRenderPass.h"

#include <stdexcept>
#include <iostream>
#include <cstring>
#include <src/renderer/vulkan/rendering/VulkanRenderPass.h>
#include <src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h>

/* Configures the render rendering with the attachments and subpasses */
MainSceneRenderPass::MainSceneRenderPass(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain) :
        VulkanRenderPassOld(device, vulkanMemory, swapChain), graphicsPipeline(nullptr) {}

void MainSceneRenderPass::init() {
    // Create the render rendering
//    renderPass = std::make_unique<VulkanRenderPass>(createRenderPass());


    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(device, AttachmentType::Color).build());
    attachments.emplace_back(VulkanAttachmentBuilder(device, AttachmentType::Depth).build());
    renderPass = std::make_unique<VulkanRenderPass>(VulkanRenderPass::Create(device, attachments));

    // Descriptor layout for this pipeline and the pool
    createBufferedDescriptorSetLayout();

    descriptorSetLayoutMaterials = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetBuilder(device)
                    .addBinding(0, DescriptorType::Texture, ShaderStage::Fragment)
                    .addBinding(1, DescriptorType::UniformBuffer, ShaderStage::Fragment)
                    .build()
    );

    descriptorSetLayoutLights = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetBuilder(device)
                    .addBinding(0, DescriptorType::UniformBuffer, ShaderStage::Fragment)
                    .build()
    );

    // Pipeline layout
    auto graphicsPipelineLayout = VulkanPipelineLayoutBuilder(device)
            .addPushConstant(sizeof(glm::mat4), 0, ShaderStage::Vertex)
            .addDescriptorSet(*descriptorSetLayoutCameraBuf)
            .addDescriptorSet(*descriptorSetLayoutLights)
            .addDescriptorSet(*descriptorSetLayoutMaterials)
            .build();

    // Pipeline creation
    auto attributeDescription = Vertex::getAttributeDescriptions();
    graphicsPipeline = std::make_unique<VulkanPipeline>(
            VulkanPipeline::Create(device,
                                   Vertex::getBindingDescription(),
                                   attributeDescription.data(),
                                   static_cast<uint32_t>(attributeDescription.size()),
                                   swapChain.getExtent(), std::move(graphicsPipelineLayout), renderPass->vk(),
                                   "base"
            ));

    // Create pipeline resources
    createUniformBuffers();
    // Create descripor pools
    createBufferedDescriptorPool();
    createBufferedDescriptorSets();

    descriptorPoolMaterials = VulkanDescriptor::createPool(device,
                                                           {
                                                                   VkDescriptorPoolSize{ // Texture
                                                                           .type = descriptorSetLayoutMaterials->getBinding(
                                                                                   0).descriptorType,
                                                                           .descriptorCount = 1024
                                                                   },
                                                                   VkDescriptorPoolSize{ // Material parameters buffer
                                                                           .type = descriptorSetLayoutMaterials->getBinding(
                                                                                   1).descriptorType,
                                                                           .descriptorCount = 1024
                                                                   }
                                                           });
    // Material descriptor sets are created at createMaterial

    createLightStructures();
}

/* Creates the layout for the pipeline (DescripotSetLayout, PushConstant)
	Creates a DescriporPool for this layout.
   */
void MainSceneRenderPass::createBufferedDescriptorSetLayout() {

    descriptorSetLayoutCameraBuf = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetBuilder(device)
                    .addBinding(0, DescriptorType::UniformBuffer, ShaderStage::VertexFragment)
                    .build()
    );
}

/* Creates the descriptor pool for the buffered uniforms. */
void MainSceneRenderPass::createBufferedDescriptorPool() {
    descriptorPoolCamera = VulkanDescriptor::createPool(device,
                                                        {
                                                                VkDescriptorPoolSize{  // Camera + worldLight
                                                                        .type=descriptorSetLayoutCameraBuf->getBinding(
                                                                                0).descriptorType,
                                                                        .descriptorCount = swapChain.size()
                                                                }
                                                        });
}

/* Creates uniform buffers for each swapchain image. */
void MainSceneRenderPass::createUniformBuffers() {

    uniformBuffers.resize(swapChain.size());

    for (size_t i = 0; i < swapChain.size(); i++) {
        uniformBuffers[i] = vulkanMemory.createUniformBuffer(
                sizeof(UniformBufferObject),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                1, false);
    }

    uboContent = UniformBufferContent<UniformBufferObject>(1);


}

/* Creates the descriptor sets from the descriptor pool */
void MainSceneRenderPass::createBufferedDescriptorSets() {
    // Create the descriptor set for each frame
    descriptorSetsCamera.resize(swapChain.size());
    for (size_t i = 0; i < descriptorSetsCamera.size(); i++) {
        descriptorSetsCamera[i] = VulkanDescriptor::allocateDescriptorSet(
                device, *descriptorSetLayoutCameraBuf,
                descriptorPoolCamera);
    }

    // Configure the descriptors of the sets
    for (size_t i = 0; i < swapChain.size(); i++) {
        VulkanDescriptor::writeDescriptorSet(device, descriptorSetsCamera[i],
                                             { // BufferInfos
                                                     {.descriptorInfo=VkDescriptorBufferInfo{ // Camera + worldLight
                                                             .buffer=uniformBuffers[i].buffer,
                                                             .offset=0,
                                                             .range=VK_WHOLE_SIZE
                                                     },
                                                             .binding=0,
                                                             .arrayElement=0,
                                                             .type=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                             .count=1
                                                     }
                                             },
                                             {} // ImageInfos
        );
    }
}

/* Creates the descriptors and buffers for the lights. */
void MainSceneRenderPass::createLightStructures() {
    // Create descriptor sets for lights
    descriptorPoolLights = VulkanDescriptor::createPool(device,
                                                        {
                                                                VkDescriptorPoolSize{
                                                                        .type = descriptorSetLayoutLights->getBinding(
                                                                                0).descriptorType,
                                                                        .descriptorCount = swapChain.size()
                                                                }
                                                        });

    // Create Light descriptor sets
    lightUniformBuffers[0] = vulkanMemory.createUniformBuffer(
            sizeof(UniformLightsObject),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    lightsUBContent[0] = UniformBufferContent<UniformLightsObject>(1);
    // Create buffer for this descriptor set
    lightUniformBuffers[1] = vulkanMemory.createUniformBuffer(
            sizeof(UniformLightsObject),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    lightsUBContent[1] = UniformBufferContent<UniformLightsObject>(1);

    UniformLightsObject *lights1 = lightsUBContent[0].at();
    lights1->lightsCount = glm::vec4(0.0f, 0, 0, 0);
    // lights1->sources[0].lightPos = glm::vec4(-0.0f, +5.0f, 0.0f, 200.0f);
    // lights1->sources[0].lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    vulkanMemory.copyDataToBuffer(lightUniformBuffers[0].buffer, lightUniformBuffers[0].memory, lights1,
                                  sizeof(UniformLightsObject));
    // Fill Lights buffer
    UniformLightsObject *lights2 = lightsUBContent[1].at();
    std::memcpy(lights2, lights1, sizeof(UniformLightsObject));
    vulkanMemory.copyDataToBuffer(lightUniformBuffers[1].buffer, lightUniformBuffers[1].memory, lights2,
                                  sizeof(UniformLightsObject));

    // Write the descriptor sets
    descriptorSetsLights.resize(swapChain.size());
    for (size_t i = 0; i < swapChain.size(); i++) {
        descriptorSetsLights[i] = VulkanDescriptor::allocateDescriptorSet(device,
                                                                          *descriptorSetLayoutLights,
                                                                          descriptorPoolLights);

        VulkanDescriptor::writeDescriptorSet(device, descriptorSetsLights[i],
                                             {
                                                     DescriptorBufferInfo{
                                                             .descriptorInfo = VkDescriptorBufferInfo{
                                                                     .buffer = lightUniformBuffers[0].buffer,
                                                                     .offset = 0,
                                                                     .range = VK_WHOLE_SIZE
                                                             },
                                                             .binding = 0,
                                                             .arrayElement = 0,
                                                             .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                             .count = 1
                                                     }
                                             },
                                             {} // Image descriptors
        );
    }
}

/* Updates the uniform buffers. */
void MainSceneRenderPass::updateUniformBuffer(uint32_t currentImage,
                                              Camera &camera, LightObject &worldLight) {
    UniformBufferObject *ubo = uboContent.at(0);
    //ubo->model = glm::translate(glm::mat4(1.0f), glm::vec3(0, +0.0f, -0.5f)) * glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo->view = camera.view;
    ubo->proj = glm::perspective(glm::radians(camera.angle),
                                 swapChain.getExtent().width /
                                 (float) swapChain.getExtent().height, camera.near, camera.far);
    ubo->proj[1][1] *= -1;
    ubo->worldLightPosition = worldLight.lightPos;
    ubo->worldLightColor = worldLight.lightColor;

    // Copy that data to the uniform buffer
    vulkanMemory.copyDataToBuffer(uniformBuffers[currentImage].buffer,
                                  uniformBuffers[currentImage].memory,
                                  uboContent.data(), uboContent.size());
}

// Rendering stuff
/* Begin the render rendering and setup all context descriptors . */
void MainSceneRenderPass::cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) {

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
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getPipeline());

    // Bind the descriptor set to the pipeline
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            graphicsPipeline->getPipelineLayout(),
                            0, 1, &descriptorSetsCamera[currentImage], 0, nullptr);
    // Bind the lights descriptor set
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            graphicsPipeline->getPipelineLayout(),
                            1, 1, &descriptorSetsLights[currentImage], 0, nullptr);
}

/* Setup all descriptors and push constants for this render object. */
void MainSceneRenderPass::cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) {

    if (robj.material < descriptorSetsMaterials.size()) {
        // Bind the material descriptor set to the pipeline // TOBE: not per renderobject
        vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                graphicsPipeline->getPipelineLayout(),
                                2, 1, &descriptorSetsMaterials[robj.material], 0, nullptr);
    } else {
        std::cerr << "MainSceneRenderPass: unknown materialRef (descriptorSet not fount) ref=" << robj.material
                  << std::endl;
    }

    // Set model matrix via push constant
    vkCmdPushConstants(cmdBuf, graphicsPipeline->getPipelineLayout(),
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(robj.modelMat), &robj.modelMat);
}

/* End this render rendering. */
void MainSceneRenderPass::cmdEnd(VkCommandBuffer &cmdBuf) {
    // Finish the render rendering
    vkCmdEndRenderPass(cmdBuf);
}

void MainSceneRenderPass::destroySwapChainDependent() {
    // These uniform buffers are per frame and therefore depend on the number of swapchain images
    for (size_t i = 0; i < uniformBuffers.size(); i++) {
        vulkanMemory.destroy(uniformBuffers[i]);
    }

    // The descriptor pool and sets depend also on the number of images in the swapchain
    vkDestroyDescriptorPool(device.vk(), descriptorPoolCamera,
                            nullptr); // this also destroys the descriptor sets of this pools
}

/* Recreates this render rendering to fit the new swap chain. */
void MainSceneRenderPass::recreate() {
    // Recreate the pipeline because the swap chain dimensions might have changed
    auto attributeDescription = Vertex::getAttributeDescriptions();
    *graphicsPipeline = VulkanPipeline::Create(device,
                                               Vertex::getBindingDescription(),
                                               attributeDescription.data(),
                                               static_cast<uint32_t>(attributeDescription.size()),
                                               swapChain.getExtent(), graphicsPipeline->releasePipelineLayout(),
                                               renderPass->vk(),
                                               "base"
    );

    // The UniformBuffers depend on the ammount of swap chain images
    createUniformBuffers();

    // Recreate the descriptors
    createBufferedDescriptorPool();
    createBufferedDescriptorSets();
}

void MainSceneRenderPass::destroy() {

    // this also destroys the descriptor sets of this pool
    vkDestroyDescriptorPool(device.vk(), descriptorPoolMaterials, nullptr);
    vkDestroyDescriptorPool(device.vk(), descriptorPoolLights, nullptr);

    for (size_t i = 0; i < materialUniformBuffers.size(); i++) {
        vulkanMemory.destroy(materialUniformBuffers[i]);
    }
    for (size_t i = 0; i < lightUniformBuffers.size(); i++) {
        vulkanMemory.destroy(lightUniformBuffers[i]);
    }

    textures.clear();

    uboContent.destroy();
}

////////////////////////////////////////////////////////////////////
MaterialRef MainSceneRenderPass::createMaterial(const TexturePhongMaterial &material) {
    // Load texture
    const VulkanTexture &texture = textures.emplace(material.textureFile, VulkanTexture::createTexture(
            device, vulkanMemory, "textures/" + material.textureFile)).first->second;

    // Create and fill material buffer
    // TODO, should not be host visible -> staging
    VulkanUniformBuffer materialBuffer = vulkanMemory.createUniformBuffer(
            sizeof(UniformMaterialObject),
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );
    materialUniformBuffers.emplace_back(materialBuffer);
    materialUBContent.emplace_back(
            UniformBufferContent<UniformMaterialObject>(1)
    );
    UniformMaterialObject *matBufferPointer = materialUBContent[materialUBContent.size() - 1].at();
    matBufferPointer->shininess = material.shininess;
    vulkanMemory.copyDataToBuffer(materialBuffer.buffer, materialBuffer.memory,
                                  matBufferPointer, sizeof(UniformMaterialObject));

    // Create and write descriptor set for this material
    descriptorSetsMaterials.emplace_back(
            VulkanDescriptor::allocateDescriptorSet(device,
                                                    *descriptorSetLayoutMaterials, descriptorPoolMaterials)
    );
    MaterialRef ref = descriptorSetsMaterials.size() - 1;
    VulkanDescriptor::writeDescriptorSet(device, descriptorSetsMaterials[ref],
                                         {
                                                 DescriptorBufferInfo{
                                                         .descriptorInfo = VkDescriptorBufferInfo{
                                                                 .buffer = materialBuffer.buffer,
                                                                 .offset = 0,
                                                                 .range = VK_WHOLE_SIZE
                                                         },
                                                         .binding = 1,
                                                         .arrayElement = 0,
                                                         .type  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                         .count = 1
                                                 }
                                         },
                                         {DescriptorImageInfo{.descriptorInfo = VkDescriptorImageInfo{
                                                 .sampler = texture.getSampler(),
                                                 .imageView = texture.getImageView().vk(),
                                                 .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                         },
                                                 .binding = 0,
                                                 .arrayElement = 0,
                                                 .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                 .count = 1
                                         }
                                         }
    );

    return ref;
}
