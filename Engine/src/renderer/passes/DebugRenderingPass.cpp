#include "DebugRenderingPass.h"
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

DebugRenderingPass
DebugRenderingPass::Create(const VulkanContext &context, const VulkanRenderPass& pass, uint32_t width, uint32_t height) {
    auto ret = DebugRenderingPass(context, pass);
    ret.init(width, height);
    return ret;
}

DebugRenderingPass::DebugRenderingPass(DebugRenderingPass &&o) noexcept:
        context(o.context), renderPass(o.renderPass),
        descriptorPool(std::move(o.descriptorPool)),
        cameraDescriptorLayout(std::move(o.cameraDescriptorLayout)),
        pipeline(std::move(o.pipeline)),
        perFrameDescriptorSets(std::move(o.perFrameDescriptorSets)),
        perFrameUniformBuffers(std::move(o.perFrameUniformBuffers)),
        uboContent(std::move(o.uboContent)),
        viewportSize(std::move(o.viewportSize)){}


void DebugRenderingPass::createStandardPipeline() {
    cameraDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(context.getDevice())
                    .addBinding(0, Renderer::ShaderBindingType::UniformBuffer, Renderer::ShaderStage::Vertex)
                    .build());

    VulkanPipelineLayout pipelineLayout = VulkanPipelineLayoutBuilder(context.getDevice())
            .addDescriptorSet(*cameraDescriptorLayout) // set = 0
            .build();

    pipeline = std::make_unique<VulkanPipeline>(
            VulkanPipelineBuilder(
                    context.getDevice(), renderPass,
                    std::move(pipelineLayout),
                    VertexAttributeBuilder(0, sizeof(VertexPC), InputRate::Vertex)
                            .addAttribute(0, VertexFormat::RGB_FLOAT, offsetof(VertexPCU, pos))
                            .addAttribute(1, VertexFormat::RGBA_FLOAT, offsetof(VertexPCU, color))
                            .build(),
                    "ENGINE_DEBUG")
                    .setFragmentShader("ENGINE_DEBUG")
                    .setTopology(Renderer::Topology::LineList)
                    .setPolygonMode(Renderer::PolygonMode::Line)
                    .setCullFace(Renderer::CullFace::CCLW)
                    .setDepthTestEnabled(false)
                    .setDepthCompare(Renderer::CompareOp::Less)
                    .setAlphaBlendingEnabled(false)
                    .build());

    descriptorPool = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(context.getDevice())
                    .addDescriptor(cameraDescriptorLayout->getBinding(0).descriptorType,
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

void DebugRenderingPass::init(uint32_t width, uint32_t height) {
    resizeAttachments(width, height);

    createStandardPipeline();
}

void DebugRenderingPass::updateUniformBuffer(const glm::mat4 &viewMat, const CameraComponent &camera,
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

void DebugRenderingPass::begin(const glm::mat4 &viewMat, const CameraComponent &camera) {
    updateUniformBuffer(viewMat, camera, viewportSize);

    auto &commandBuffer = context.getCurrentPrimaryCommandBuffer();

    // Now vkCmd... can be written, to define the draw calls
    // Bind the pipeline as a graphics pipeline
    vkCmdBindPipeline(commandBuffer.vk(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->getPipeline());


    // Bind the descriptor set to the pipeline
    auto cameraDescriptor = perFrameDescriptorSets[context.getCurrentFrame()].vk();
    vkCmdBindDescriptorSets(commandBuffer.vk(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline->getPipelineLayout(),
                            0, 1, &cameraDescriptor, 0, nullptr);

}

void DebugRenderingPass::resizeAttachments(uint32_t width, uint32_t height) {
    viewportSize = glm::vec2{width, height};
}

void
DebugRenderingPass::drawLines(const VulkanBuffer &vertexBuffer, uint32_t vertexCount) {
    auto &commandBuffer = context.getCurrentPrimaryCommandBuffer();

    vkCmdSetLineWidth(commandBuffer.vk(), 2.0f);

    VkBuffer vertexBuffers[]{vertexBuffer.vk()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer.vk(), 0, 1, vertexBuffers, offsets);
    vkCmdDraw(commandBuffer.vk(), vertexCount, 1, 0, 0);
}
