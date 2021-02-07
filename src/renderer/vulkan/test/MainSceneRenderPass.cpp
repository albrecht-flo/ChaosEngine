#include "MainSceneRenderPass.h"

#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"
#include "src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"

#include <stdexcept>
#include <iostream>
#include <cstring>

/* Configures the render rendering with the attachments and subpasses */
MainSceneRenderPass::MainSceneRenderPass(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain) :
        VulkanRenderPassOld(device, vulkanMemory, swapChain), graphicsPipeline(nullptr) {
    vertex_3P_3C_3N_2U = VertexAttributeBuilder(0, sizeof(Vertex), InputRate::Vertex)
            .addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            .addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
            .addAttribute(2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
            .addAttribute(3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)).build();
}

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
            VulkanDescriptorSetLayoutBuilder(device)
                    .addBinding(0, DescriptorType::Texture, ShaderStage::Fragment)
                    .addBinding(1, DescriptorType::UniformBuffer, ShaderStage::Fragment)
                    .build()
    );

    descriptorSetLayoutLights = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(device)
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
    graphicsPipeline = std::make_unique<VulkanPipeline>(
            VulkanPipelineBuilder(device, *renderPass, std::move(graphicsPipelineLayout), vertex_3P_3C_3N_2U, "base")
                    .setTopology(Topology::TriangleList)
                    .setPolygonMode(PolygonMode::Fill)
                    .setCullFace(CullFace::CCLW)
                    .setDepthTestEnabled(true)
                    .setDepthCompare(CompareOp::Less)
                    .build()
    );

    // Create pipeline resources
    createUniformBuffers();
    createBufferedDescriptorSets();

    descriptorPoolMaterials = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(device)
                    .addDescriptor(descriptorSetLayoutMaterials->getBinding(0).descriptorType, 1024)
                    .addDescriptor(descriptorSetLayoutMaterials->getBinding(1).descriptorType, 1024)
                    .setMaxSets(1024)
                    .build()
    );
    // Material descriptor sets are created at createMaterial

    createLightStructures();
}

/* Creates the layout for the pipeline (DescripotSetLayout, PushConstant)
	Creates a DescriporPool for this layout.
   */
void MainSceneRenderPass::createBufferedDescriptorSetLayout() {

    descriptorSetLayoutCameraBuf = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(device)
                    .addBinding(0, DescriptorType::UniformBuffer, ShaderStage::VertexFragment)
                    .build()
    );
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

/* Creates the descriptor pool and the sets */
void MainSceneRenderPass::createBufferedDescriptorSets() {
//// TODO: Remove swapchain size dependency
    descriptorPoolCamera = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(device)
                    .addDescriptor(descriptorSetLayoutCameraBuf->getBinding(0).descriptorType, swapChain.size())
                    .setMaxSets(swapChain.size())
                    .build()
    );

    // Create the descriptor set for each frame
    descriptorSetsCamera.reserve(swapChain.size());
    for (size_t i = 0; i < swapChain.size(); i++) {
        descriptorSetsCamera.emplace_back(descriptorPoolCamera->allocate(*descriptorSetLayoutCameraBuf));
        descriptorSetsCamera[i].startWriting().writeBuffer(0, uniformBuffers[i].buffer).commit();
    }
}

/* Creates the descriptors and buffers for the lights. */
void MainSceneRenderPass::createLightStructures() {
    // Create descriptor sets for lights
    descriptorPoolLights = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(device)
                    .addDescriptor(descriptorSetLayoutLights->getBinding(0).descriptorType, swapChain.size())
                    .setMaxSets(swapChain.size())
                    .build()
    );

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
    descriptorSetsLights.reserve(swapChain.size());
    for (size_t i = 0; i < swapChain.size(); i++) {
        descriptorSetsLights.emplace_back(descriptorPoolLights->allocate(*descriptorSetLayoutLights));
        descriptorSetsLights[i].startWriting().writeBuffer(0, lightUniformBuffers[0].buffer).commit();
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
void MainSceneRenderPass::cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer,
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
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(viewportWidth);
    viewport.height = static_cast<float>(viewportHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {viewportWidth, viewportHeight};
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    // Bind the descriptor set to the pipeline
    auto cameraDescriptor = descriptorSetsCamera[currentImage].vk();
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            graphicsPipeline->getPipelineLayout(),
                            0, 1, &cameraDescriptor, 0, nullptr);
    // Bind the lights descriptor set
    auto lightsDescriptor = descriptorSetsLights[currentImage].vk();
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            graphicsPipeline->getPipelineLayout(),
                            1, 1, &lightsDescriptor, 0, nullptr);
}

/* Setup all descriptors and push constants for this render object. */
void MainSceneRenderPass::cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) {

    if (robj.material < descriptorSetsMaterials.size()) {
        // Bind the material descriptor set to the pipeline // TOBE: not per renderobject
        auto materialDescriptor = descriptorSetsMaterials[robj.material].vk();
        vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                graphicsPipeline->getPipelineLayout(),
                                2, 1, &materialDescriptor, 0, nullptr);
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
}

/* Recreates this render rendering to fit the new swap chain. */
void MainSceneRenderPass::recreate() {
    // The UniformBuffers depend on the ammount of swap chain images
    createUniformBuffers();

}

void MainSceneRenderPass::destroy() {

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
    descriptorSetsMaterials.emplace_back(descriptorPoolMaterials->allocate(*descriptorSetLayoutMaterials));
    MaterialRef ref = descriptorSetsMaterials.size() - 1;
    descriptorSetsMaterials[ref].startWriting()
            .writeBuffer(1, materialBuffer.buffer)
            .writeImageSampler(0, texture.getSampler(), texture.getImageView().vk(),
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            .commit();
    return ref;
}
