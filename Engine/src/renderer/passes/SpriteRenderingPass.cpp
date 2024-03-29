#include "SpriteRenderingPass.h"
#include "Engine/src/core/utils/Logger.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSetLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPoolBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "Engine/src/renderer/vulkan/api/VulkanRenderMesh.h"

using namespace Renderer;

// ------------------------------------ Class Members ------------------------------------------------------------------

SpriteRenderingPass
SpriteRenderingPass::Create(const VulkanContext &context, uint32_t width, uint32_t height) {
    assert("Viewport size MUST be greater than 0!" && width > 0 && height > 0);
    auto ret = SpriteRenderingPass(context);
    ret.init(width, height);
    return ret;
}

SpriteRenderingPass::SpriteRenderingPass(SpriteRenderingPass &&o) noexcept:
        context(o.context), opaquePass(std::move(o.opaquePass)),
        framebuffer(std::move(o.framebuffer)),
        descriptorPool(std::move(o.descriptorPool)),
        cameraDescriptorLayout(std::move(o.cameraDescriptorLayout)),
        materialDescriptorLayout(std::move(o.materialDescriptorLayout)),
        pipeline(std::move(o.pipeline)),
        perFrameDescriptorSets(std::move(o.perFrameDescriptorSets)),
        perFrameUniformBuffers(std::move(o.perFrameUniformBuffers)),
        uboContent(std::move(o.uboContent)),
        viewportSize(std::move(o.viewportSize)) {}

void SpriteRenderingPass::createAttachments(uint32_t width, uint32_t height) {
    framebuffer = std::make_unique<VulkanFramebuffer>(opaquePass->createFrameBuffer(
            {FramebufferAttachmentInfo{AttachmentType::Color, AttachmentFormat::U_R8G8B8A8, 0},
             FramebufferAttachmentInfo{AttachmentType::Depth, AttachmentFormat::Auto_Depth}},
            width, height, "Sprite Pass"));
    viewportSize = {width, height};
}

void SpriteRenderingPass::createStandardPipeline() {
    cameraDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(context.getDevice())
                    .addBinding(0, Renderer::ShaderBindingType::UniformBuffer, Renderer::ShaderStage::Vertex)
                    .build());

    VulkanPipelineLayout pipelineLayout = VulkanPipelineLayoutBuilder(context.getDevice())
            .addPushConstant(sizeof(glm::mat4), 0, Renderer::ShaderStage::Vertex)
            .addDescriptorSet(*cameraDescriptorLayout) // set = 0
            .build();

    pipeline = std::make_unique<VulkanPipeline>(
            VulkanPipelineBuilder(context.getDevice(), *opaquePass,
                                  std::move(pipelineLayout),
                                  VulkanRenderMesh::vertex_3P_3C_3N_2U,
                                  "ENGINE_2DBase")
                    .setFragmentShader("ENGINE_2DBase")
                    .setTopology(Renderer::Topology::TriangleList)
                    .setPolygonMode(Renderer::PolygonMode::Fill)
                    .setCullFace(Renderer::CullFace::CCLW)
                    .setDepthTestEnabled(true)
                    .setDepthCompare(Renderer::CompareOp::Less)
                    .build());

    descriptorPool = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(context.getDevice())
                    .addDescriptor(cameraDescriptorLayout->getBinding(
                                           0).descriptorType,
                                   GraphicsContext::maxFramesInFlight)
                    .setMaxSets(GraphicsContext::maxFramesInFlight)
                    .build());


    perFrameUniformBuffers.reserve(GraphicsContext::maxFramesInFlight);
    for (size_t i = 0; i < perFrameUniformBuffers.capacity(); i++) {
        perFrameUniformBuffers.emplace_back(context.getMemory().createUniformBuffer(sizeof(CameraUbo), 1, false));
    }
    uboContent = std::make_unique<UniformBufferContent<CameraUbo>>(1);

    perFrameDescriptorSets.reserve(GraphicsContext::maxFramesInFlight);
    for (size_t i = 0; i < perFrameDescriptorSets.capacity(); i++) {
        perFrameDescriptorSets.emplace_back(descriptorPool->allocate(*cameraDescriptorLayout));
        perFrameDescriptorSets[i].startWriting()
                .writeBuffer(0, perFrameUniformBuffers[i].getBuffer().vk())
                .commit();
    }

}

void SpriteRenderingPass::init(uint32_t width, uint32_t height) {
    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(context, AttachmentType::Color)
                                     .build());
    attachments.emplace_back(VulkanAttachmentBuilder(context, AttachmentType::Depth).build());
    opaquePass = std::make_unique<VulkanRenderPass>(
            VulkanRenderPass::Create(context, attachments, "SpriteRenderPass-Color"));

    createAttachments(width, height);

    createStandardPipeline();
}

void SpriteRenderingPass::updateUniformBuffer(const glm::mat4 &viewMat, const CameraComponent &camera,
                                              const glm::uvec2 &viewportDimensions) {

    CameraUbo *ubo = uboContent->at(context.getCurrentFrame());
    ubo->view = viewMat;
    if (viewportDimensions.x > viewportDimensions.y) {
        float aspect = static_cast<float>(viewportDimensions.x) / static_cast<float>(viewportDimensions.y);
        ubo->proj = glm::ortho(-camera.fieldOfView * aspect, camera.fieldOfView * aspect, -camera.fieldOfView,
                               camera.fieldOfView, camera.near, camera.far);
    } else {
        float aspect = static_cast<float>(viewportDimensions.y) / static_cast<float>(viewportDimensions.x);
        ubo->proj = glm::ortho(-camera.fieldOfView, camera.fieldOfView, -camera.fieldOfView * aspect,
                               camera.fieldOfView * aspect, camera.near, camera.far);
    }
    ubo->proj[1][1] *= -1; // GLM uses OpenGL projection -> Y Coordinate needs to be flipped

    // Copy that data to the uniform buffer
    context.getMemory().copyDataToBuffer(perFrameUniformBuffers[context.getCurrentFrame()].getBuffer(),
                                         uboContent->data(), uboContent->size(), 0);
}

void SpriteRenderingPass::begin(const glm::mat4 &viewMat, const CameraComponent &camera) {
    updateUniformBuffer(viewMat, camera, viewportSize);

    auto &commandBuffer = context.getCurrentPrimaryCommandBuffer();
    // Define render rendering to draw with
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = opaquePass->vk(); // the renderpass to use
    renderPassInfo.framebuffer = framebuffer->vk();
    renderPassInfo.renderArea.offset = {0, 0}; // size of the render area ...
    renderPassInfo.renderArea.extent = VkExtent2D{viewportSize.x, viewportSize.y}; // based on swap chain

    // Define the values used for VK_ATTACHMENT_LOAD_OP_CLEAR
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {1.0f, 0.0f, 1.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer.vk(), &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE); // use commands in primary cmdbuffer

    // Now vkCmd... can be written, to define the draw calls
    // Bind the pipeline as a graphics pipeline
    vkCmdBindPipeline(commandBuffer.vk(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(viewportSize.x);
    viewport.height = static_cast<float>(viewportSize.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer.vk(), 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {context.getSwapChain().getWidth(), context.getSwapChain().getHeight()};
    vkCmdSetScissor(commandBuffer.vk(), 0, 1, &scissor);

    // Bind the descriptor set to the pipeline
    auto cameraDescriptor = perFrameDescriptorSets[context.getCurrentFrame()].vk();
    vkCmdBindDescriptorSets(commandBuffer.vk(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline->getPipelineLayout(),
                            0, 1, &cameraDescriptor, 0, nullptr);

}

void SpriteRenderingPass::end() {
    vkCmdEndRenderPass(context.getCurrentPrimaryCommandBuffer().vk());
}

void SpriteRenderingPass::resizeAttachments(uint32_t width, uint32_t height) {
    createAttachments(width, height);
}

void
SpriteRenderingPass::drawSprite(const VulkanRenderMesh &renderMesh, const glm::mat4 &modelMat,
                                const VulkanMaterialInstance &material) {
    auto &commandBuffer = context.getCurrentPrimaryCommandBuffer();

    // Bind Material TODO: The pipeline should be bound for a group of objects
    vkCmdBindPipeline(commandBuffer.vk(), VK_PIPELINE_BIND_POINT_GRAPHICS, material.getPipeline());
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(viewportSize.x);
    viewport.height = static_cast<float>(viewportSize.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer.vk(), 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {viewportSize.x, viewportSize.y};
    vkCmdSetScissor(commandBuffer.vk(), 0, 1, &scissor);

    if (material.isNonSolid()) {
        vkCmdSetLineWidth(commandBuffer.vk(), 3.0f);
    }

    // Bind Material
    auto materialDescriptorSet = material.getDescriptorSet().vk();
    vkCmdBindDescriptorSets(commandBuffer.vk(), VK_PIPELINE_BIND_POINT_GRAPHICS, material.getPipelineLayout(), 1, 1,
                            &materialDescriptorSet, 0, nullptr);

    // Set model matrix via push constant
    vkCmdPushConstants(commandBuffer.vk(), pipeline->getPipelineLayout(),
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(modelMat), &modelMat);

    VkBuffer vertexBuffers[]{dynamic_cast<const VulkanBuffer *>(renderMesh.getVertexBuffer())->vk()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer.vk(), 0, 1, vertexBuffers, offsets);
    auto vIndexBuffer = dynamic_cast<const VulkanBuffer *>(renderMesh.getIndexBuffer())->vk();
    vkCmdBindIndexBuffer(commandBuffer.vk(), vIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    // Draw a fullscreen quad and composite the final image
    vkCmdDrawIndexed(commandBuffer.vk(), renderMesh.getIndexCount(), 1, 0, 0, 0);
}
