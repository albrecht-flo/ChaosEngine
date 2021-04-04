#include "PostProcessingPass.h"
#include "Engine/src/core/Utils/Logger.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanVertexInput.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSetLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPoolBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "Engine/src/renderer/vulkan/image/VulkanTexture.h"

#include <array>

using namespace Renderer;

// ------------------------------------ Class Members ------------------------------------------------------------------

PostProcessingPass PostProcessingPass::Create(const VulkanContext &context, const VulkanFramebuffer &previousPassFB,
                                              bool renderToSwapchain, uint32_t width, uint32_t height) {
    assert("If the PostProcessingPass does NOT render to the swapchain, a width and height have to be supplied" &&
           (renderToSwapchain || (width != 0 && height != 0)));
    PostProcessingPass postProcessingPass(context, renderToSwapchain);
    postProcessingPass.init(width, height, previousPassFB);
    return postProcessingPass;
}

PostProcessingPass::PostProcessingPass(PostProcessingPass &&o) noexcept
        : context(o.context), renderToSwapchain(o.renderToSwapchain), renderPass(std::move(o.renderPass)),
          swapChainFrameBuffers(std::move(o.swapChainFrameBuffers)),
          descriptorSetLayout(std::move(o.descriptorSetLayout)),
          postprocessingPipeline(std::move(o.postprocessingPipeline)),
          descriptorPool(std::move(o.descriptorPool)),
          quadBuffer(std::move(o.quadBuffer)),
          perFrameDescriptorSet(std::move(o.perFrameDescriptorSet)),
          perFrameUniformBuffer(std::move(o.perFrameUniformBuffer)),
          uboContent(std::move(o.uboContent)),
          viewportSize(std::move(o.viewportSize)) {}

void
PostProcessingPass::init(uint32_t width, uint32_t height, const VulkanFramebuffer &previousPassFb) {

    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(context.getDevice(), AttachmentType::Color)
                                     .build());
    renderPass = std::make_unique<VulkanRenderPass>(
            VulkanRenderPass::Create(context, attachments, "PostProcessingRenderPass"));

    createAttachments(width, height);

    struct Vertex {
        glm::vec3 pos;
        glm::vec2 uv;
    };
    Vertex quad[] = {
            Vertex{glm::vec3(-1, -1, 0), glm::vec2(0, 1)},
            Vertex{glm::vec3(+1, -1, 0), glm::vec2(1, 1)},
            Vertex{glm::vec3(+1, +1, 0), glm::vec2(1, 0)},
            Vertex{glm::vec3(+1, +1, 0), glm::vec2(1, 0)},
            Vertex{glm::vec3(-1, +1, 0), glm::vec2(0, 0)},
            Vertex{glm::vec3(-1, -1, 0), glm::vec2(0, 1)},
    };
    quadBuffer = std::make_unique<VulkanBuffer>(context.getMemory().createInputBuffer(6 * sizeof(quad[0]), quad,
                                                                                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
    auto vertex_3P_2U = std::make_unique<VulkanVertexInput>(
            VertexAttributeBuilder(0, sizeof(Vertex), InputRate::Vertex)
                    .addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
                    .addAttribute(1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)).build());


    descriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(context.getDevice())
                    .addBinding(0, ShaderBindingType::UniformBuffer, ShaderStage::Fragment) // Configuration
                    .addBinding(1, ShaderBindingType::TextureSampler, ShaderStage::Fragment) // Color
                    .addBinding(2, ShaderBindingType::TextureSampler, ShaderStage::Fragment) // Depth
                    .build());

    VulkanPipelineLayout pipelineLayout = VulkanPipelineLayoutBuilder(context.getDevice())
            .addDescriptorSet(*descriptorSetLayout) // set = 0
            .build();

    postprocessingPipeline = std::make_unique<VulkanPipeline>(
            VulkanPipelineBuilder(context.getDevice(), *renderPass, std::move(pipelineLayout), *vertex_3P_2U,
                                  "2DPostProcessing")
                    .setTopology(Renderer::Topology::TriangleList)
                    .setPolygonMode(Renderer::PolygonMode::Fill)
                    .setCullFace(Renderer::CullFace::CCLW)
                    .setDepthTestEnabled(false)
                    .build());

    descriptorPool = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(context.getDevice())
                    .addDescriptor(descriptorSetLayout->getBinding(0).descriptorType,
                                   GraphicsContext::maxFramesInFlight)
                    .addDescriptor(descriptorSetLayout->getBinding(1).descriptorType,
                                   GraphicsContext::maxFramesInFlight * 2)
                    .setMaxSets(GraphicsContext::maxFramesInFlight * 3)
                    .build());

    perFrameUniformBuffer = std::make_unique<VulkanUniformBuffer>(std::move(
            context.getMemory().createUniformBuffer(sizeof(ShaderConfig), 1, false)));
    uboContent = std::make_unique<UniformBufferContent<ShaderConfig>>(1);
    uboContent->at(0)->cameraNear = ShaderConfig{}.cameraNear;
    uboContent->at(0)->cameraFar = ShaderConfig{}.cameraFar;

    perFrameDescriptorSet = std::make_unique<VulkanDescriptorSet>(descriptorPool->allocate(*descriptorSetLayout));
    perFrameDescriptorSet->startWriting().writeBuffer(0, perFrameUniformBuffer->getBuffer().vk()).commit();
    writeDescriptorSet(previousPassFb);
}

void PostProcessingPass::createAttachments(uint32_t width, uint32_t height) {
    if (renderToSwapchain) {
        swapChainFrameBuffers = context.getSwapChain().createFramebuffers(*renderPass);
        viewportSize = {context.getSwapChain().getWidth(), context.getSwapChain().getHeight()};
    } else {
        auto fb = renderPass->createFrameBuffer(
                {FramebufferAttachmentInfo{AttachmentType::Color, AttachmentFormat::U_R8G8B8A8}}, width, height,
                "Post Processing");
        if (swapChainFrameBuffers.empty())
            swapChainFrameBuffers.emplace_back(std::move(fb));
        else
            swapChainFrameBuffers[0] = std::move(fb);
        viewportSize = {width, height};
    }
}

void PostProcessingPass::writeDescriptorSet(const VulkanFramebuffer &previousPassFB) {
    const auto &depthTex = dynamic_cast<const VulkanTexture &>(
            previousPassFB.getAttachmentTexture(AttachmentType::Depth, 0));
    const auto &colorTex = dynamic_cast<const VulkanTexture &>(
            previousPassFB.getAttachmentTexture(AttachmentType::Color, 0));

    // Fill the descriptor set
    perFrameDescriptorSet->startWriting()
            .writeImageSampler(1, colorTex.getSampler(), colorTex.getImageView(), colorTex.getImageLayout())
            .writeImageSampler(2, depthTex.getSampler(), depthTex.getImageView(), depthTex.getImageLayout())
            .commit();
}

void PostProcessingPass::draw() {
    assert("No postprocessing configured" &&
           !(static_cast<ShaderConfig *>(uboContent->at(0))->cameraNear == 0.0f &&
             static_cast<ShaderConfig *>(uboContent->at(0))->cameraFar == 0.0f));

    auto cmdBuf = context.getCurrentPrimaryCommandBuffer().vk();

    // Define render rendering to draw with
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->vk(); // the renderpass to use
    renderPassInfo.framebuffer =
            swapChainFrameBuffers[renderToSwapchain ? context.getCurrentSwapChainFrame() : 0].vk(); // the attachment
    renderPassInfo.renderArea.offset = {0, 0}; // size of the render area ...
    renderPassInfo.renderArea.extent = renderToSwapchain ? context.getSwapChain().getExtent() :
                                       VkExtent2D{swapChainFrameBuffers[0].getWidth(),
                                                  swapChainFrameBuffers[0].getHeight()}; // based on swap chain

    // Define the values used for VK_ATTACHMENT_LOAD_OP_CLEAR
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // use commands in primary cmdbuffer

    // Now vkCmd... can be written do define the draw call
    // Bind the pipeline as a graphics pipeline
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, postprocessingPipeline->getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(viewportSize.x);
    viewport.height = static_cast<float>(viewportSize.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

    VkRect2D scissor = {}; // !!! Important, otherwise ImGui sets this
    scissor.offset = {0, 0};
    scissor.extent = {viewportSize.x, viewportSize.y};
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    // Bind the descriptor set to the pipeline
    auto desc = perFrameDescriptorSet->vk();
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            postprocessingPipeline->getPipelineLayout(),
                            0, 1, &desc, 0, nullptr);

    VkBuffer vertexBuffers[]{quadBuffer->vk()};
    VkDeviceSize offsets[]{0};
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertexBuffers, offsets);
    // Draw a fullscreen quad and composite the final image
    vkCmdDraw(cmdBuf, 6, 1, 0, 0);

    vkCmdEndRenderPass(cmdBuf);
}

void PostProcessingPass::resizeAttachments(const VulkanFramebuffer &framebuffer, uint32_t width, uint32_t height) {
    assert("If the PostProcessingPass does NOT render to the swapchain, a width and height have to be supplied" &&
           (renderToSwapchain || (width != 0 && height != 0)));
    createAttachments(width, height);

    writeDescriptorSet(framebuffer);
}

void PostProcessingPass::updateConfiguration(const PostProcessingPass::PostProcessingConfiguration &configuration) {
    ShaderConfig *ubo = uboContent->at(0);

    ubo->cameraNear = configuration.camera.near;
    ubo->cameraFar = configuration.camera.far;
    // Copy that data to the uniform buffer
    context.getMemory().copyDataToBuffer(perFrameUniformBuffer->getBuffer(), uboContent->data(), uboContent->size(), 0);
}
