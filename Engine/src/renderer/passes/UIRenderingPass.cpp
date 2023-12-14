#include "UIRenderingPass.h"
#include "Engine/src/core/utils/Logger.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSetLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPoolBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "Engine/src/core/renderSystem/UIRenderSubSystem.h"

using namespace ChaosEngine;
using namespace Renderer;

// ------------------------------------ Class Members ------------------------------------------------------------------

UIRenderingPass
UIRenderingPass::Create(const VulkanContext &context, uint32_t width, uint32_t height) {
    assert("Viewport size MUST be greater than 0!" && width > 0 && height > 0);
    auto ret = UIRenderingPass(context);
    ret.init(width, height);
    return ret;
}

UIRenderingPass::UIRenderingPass(UIRenderingPass &&o) noexcept:
        context(o.context), opaquePass(std::move(o.opaquePass)),
        framebuffer(std::move(o.framebuffer)),
        descriptorPool(std::move(o.descriptorPool)),
        canvasDescriptorLayout(std::move(o.canvasDescriptorLayout)),
        materialDescriptorLayout(std::move(o.materialDescriptorLayout)),
        pipeline(std::move(o.pipeline)),
        perFrameDescriptorSets(std::move(o.perFrameDescriptorSets)),
        perFrameUniformBuffers(std::move(o.perFrameUniformBuffers)),
        uboContent(std::move(o.uboContent)),
        viewportSize(std::move(o.viewportSize)) {}

void UIRenderingPass::createAttachments(uint32_t width, uint32_t height) {
    framebuffer = std::make_unique<VulkanFramebuffer>(opaquePass->createFrameBuffer(
            {FramebufferAttachmentInfo{AttachmentType::Color, AttachmentFormat::U_R8G8B8A8, 0},
             FramebufferAttachmentInfo{AttachmentType::Depth, AttachmentFormat::Auto_Depth}},
            width, height, "UI Pass"));
    viewportSize = {width, height};
}

void UIRenderingPass::createStandardPipeline() {
    canvasDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(context.getDevice())
                    .addBinding(0, Renderer::ShaderBindingType::UniformBuffer, Renderer::ShaderStage::Vertex)
                    .build());

    VulkanPipelineLayout pipelineLayout = VulkanPipelineLayoutBuilder(context.getDevice())
            .addPushConstant(sizeof(glm::mat4), 0, Renderer::ShaderStage::Vertex)
            .addDescriptorSet(*canvasDescriptorLayout) // set = 0
            .build();

    pipeline = std::make_unique<VulkanPipeline>(
            VulkanPipelineBuilder(
                    context.getDevice(), *opaquePass,
                    std::move(pipelineLayout),
                    VertexAttributeBuilder(0, sizeof(VertexPCU), InputRate::Vertex)
                            .addAttribute(0, VertexFormat::RGB_FLOAT, offsetof(VertexPCU, pos))
                            .addAttribute(1, VertexFormat::RGBA_FLOAT, offsetof(VertexPCU, color))
                            .addAttribute(2, VertexFormat::RG_FLOAT, offsetof(VertexPCU, uv))
                            .build(),
                    "ENGINE_UIBase")
                    .setFragmentShader("ENGINE_UIBase")
                    .setTopology(Renderer::Topology::TriangleList)
                    .setPolygonMode(Renderer::PolygonMode::Fill)
                    .setCullFace(Renderer::CullFace::CCLW)
                    .setDepthTestEnabled(false)
                    .setDepthCompare(Renderer::CompareOp::Less)
                    .setAlphaBlendingEnabled(true)
                    .build());

    descriptorPool = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(context.getDevice())
                    .addDescriptor(canvasDescriptorLayout->getBinding(0).descriptorType,
                                   GraphicsContext::maxFramesInFlight)
                    .setMaxSets(GraphicsContext::maxFramesInFlight)
                    .build());


    perFrameUniformBuffers.reserve(GraphicsContext::maxFramesInFlight);
    for (size_t i = 0; i < perFrameUniformBuffers.capacity(); i++) {
        perFrameUniformBuffers.emplace_back(context.getMemory().createUniformBuffer(sizeof(CanvasUbo), 1, false));
    }
    uboContent = std::make_unique<UniformBufferContent<CanvasUbo>>(1);

    perFrameDescriptorSets.reserve(GraphicsContext::maxFramesInFlight);
    for (size_t i = 0; i < perFrameDescriptorSets.capacity(); i++) {
        perFrameDescriptorSets.emplace_back(descriptorPool->allocate(*canvasDescriptorLayout));
        perFrameDescriptorSets[i].startWriting()
                .writeBuffer(0, perFrameUniformBuffers[i].getBuffer().vk())
                .commit();
    }

}

void UIRenderingPass::init(uint32_t width, uint32_t height) {
    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(context, AttachmentType::Color).build());
    attachments.emplace_back(VulkanAttachmentBuilder(context, AttachmentType::Depth).build());
    opaquePass = std::make_unique<VulkanRenderPass>(
            VulkanRenderPass::Create(context, attachments, "UIRenderPass-Color"));

    createAttachments(width, height);

    createStandardPipeline();
}

void UIRenderingPass::updateUniformBuffer(const glm::mat4 &viewMat, const glm::uvec2 &viewportDimensions) {

    CanvasUbo *ubo = uboContent->at(context.getCurrentFrame());
    ubo->view = viewMat;
    ubo->proj = glm::ortho(0.0f, (float) viewportDimensions.x,
                           0.0f, (float) viewportDimensions.y,
                           -1000.0f, 0.0f);
    ubo->proj[1][1] *= -1; // GLM uses OpenGL projection -> Y Coordinate needs to be flipped

    // Copy that data to the uniform buffer
    context.getMemory().copyDataToBuffer(perFrameUniformBuffers[context.getCurrentFrame()].getBuffer(),
                                         uboContent->data(), uboContent->size(), 0);
}

void UIRenderingPass::begin(const glm::mat4 &viewMat) {
    updateUniformBuffer(viewMat, viewportSize);

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
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f};
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

void UIRenderingPass::end() {
    vkCmdEndRenderPass(context.getCurrentPrimaryCommandBuffer().vk());
}

void UIRenderingPass::resizeAttachments(uint32_t width, uint32_t height) {
    createAttachments(width, height);
}

void
UIRenderingPass::drawUI(const VulkanBuffer &vertexBuffer, const VulkanBuffer &indexBuffer,
                        uint32_t indexCount, uint32_t indexOffset,
                        const glm::mat4 &modelMat, const VulkanMaterialInstance &material) {
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

    VkBuffer vertexBuffers[]{vertexBuffer.vk()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer.vk(), 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer.vk(), indexBuffer.vk(), 0, VK_INDEX_TYPE_UINT32);
    // Draw a fullscreen quad and composite the final image
    vkCmdDrawIndexed(commandBuffer.vk(), indexCount, 1, indexOffset, 0, 0);
}
