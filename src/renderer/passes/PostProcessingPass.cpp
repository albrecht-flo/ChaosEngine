#include "PostProcessingPass.h"
#include "src/renderer/vulkan/pipeline/VulkanVertexInput.h"
#include "src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorSetLayoutBuilder.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorPoolBuilder.h"
#include "src/renderer/vulkan/pipeline/VulkanPipelineLayoutBuilder.h"
#include "src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"

static std::vector<VulkanFramebuffer>
createSwapChainFrameBuffers(const VulkanDevice &device, const VulkanSwapChain &swapChain,
                            const VulkanRenderPass &renderPass) {
    std::vector<VulkanFramebuffer> swapChainFramebuffers;
    swapChainFramebuffers.reserve(swapChain.size());
    for (uint32_t i = 0; i < swapChain.size(); i++) {
        swapChainFramebuffers.emplace_back(
                renderPass.createFrameBuffer(
                        {swapChain.getImageViews()[i].vk()},
                        swapChain.getExtent()
                ));
    }
    return std::move(swapChainFramebuffers);
}

// ------------------------------------ Class Members ------------------------------------------------------------------

PostProcessingPass PostProcessingPass::Create(const VulkanContext &context, const VulkanImageBuffer &colorBuffer,
                                              const VulkanImageBuffer &depthBuffer) {
    PostProcessingPass postProcessingPass(context);
    postProcessingPass.init(colorBuffer, depthBuffer);
    return std::move(postProcessingPass);
}

PostProcessingPass::PostProcessingPass(PostProcessingPass &&o) noexcept
        : context(o.context), renderPass(std::move(o.renderPass)),
          swapChainFrameBuffers(std::move(o.swapChainFrameBuffers)),
          descriptorSetLayout(std::move(o.descriptorSetLayout)),
          postprocessingPipeline(std::move(o.postprocessingPipeline)),
          descriptorPool(std::move(o.descriptorPool)),
          quadBuffer(std::move(o.quadBuffer)),
          colorBufferSampler(std::move(o.colorBufferSampler)),
          depthBufferSampler(std::move(o.depthBufferSampler)),
          perFrameDescriptorSet(std::move(o.perFrameDescriptorSet)),
          perFrameUniformBuffer(std::move(o.perFrameUniformBuffer)),
          uboContent(std::move(o.uboContent)) {}

void PostProcessingPass::init(const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer) {

    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(context.getDevice(), AttachmentType::Color)
                                     .loadStore(AttachmentLoadOp::Clear, AttachmentStoreOp::Store)
                                     .layoutInitFinal(VK_IMAGE_LAYOUT_UNDEFINED,
                                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                                     .build());
    renderPass = std::make_unique<VulkanRenderPass>(VulkanRenderPass::Create(context.getDevice(), attachments));

    swapChainFrameBuffers = createSwapChainFrameBuffers(context.getDevice(), context.getSwapChain(), *renderPass);

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
                    .addBinding(0, DescriptorType::UniformBuffer, ShaderStage::Fragment) // Configuration
                    .addBinding(1, DescriptorType::Texture, ShaderStage::Fragment) // Color
                    .addBinding(2, DescriptorType::Texture, ShaderStage::Fragment) // Depth
                    .build());

    VulkanPipelineLayout pipelineLayout = VulkanPipelineLayoutBuilder(context.getDevice())
            .addDescriptorSet(*descriptorSetLayout) // set = 0
            .build();

    postprocessingPipeline = std::make_unique<VulkanPipeline>(
            VulkanPipelineBuilder(context.getDevice(), *renderPass, std::move(pipelineLayout), *vertex_3P_2U,
                                  "2DPostProcessing")
                    .setTopology(Topology::TriangleList)
                    .setPolygonMode(PolygonMode::Fill)
                    .setCullFace(CullFace::CCLW)
                    .setDepthTestEnabled(false)
                    .build());

    descriptorPool = std::make_unique<VulkanDescriptorPool>(
            VulkanDescriptorPoolBuilder(context.getDevice())
                    .addDescriptor(descriptorSetLayout->getBinding(0).descriptorType, VulkanContext::maxFramesInFlight)
                    .addDescriptor(descriptorSetLayout->getBinding(1).descriptorType,
                                   VulkanContext::maxFramesInFlight * 2)
                    .setMaxSets(VulkanContext::maxFramesInFlight * 3)
                    .build());

    perFrameUniformBuffer = std::make_unique<VulkanUniformBuffer>(context.getMemory().createUniformBuffer(
            sizeof(ShaderConfig),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            1, false));
    uboContent = UniformBufferContent<ShaderConfig>(1);
    uboContent.at(0)->cameraNear = ShaderConfig{}.cameraNear;
    uboContent.at(0)->cameraFar = ShaderConfig{}.cameraFar;

    colorBufferSampler = std::make_unique<VulkanSampler>(VulkanSampler::create(context.getDevice(), VK_FILTER_LINEAR));
    depthBufferSampler = std::make_unique<VulkanSampler>(VulkanSampler::create(context.getDevice(), VK_FILTER_LINEAR));

    perFrameDescriptorSet = std::make_unique<VulkanDescriptorSet>(descriptorPool->allocate(*descriptorSetLayout));
    perFrameDescriptorSet->startWriting().writeBuffer(0, perFrameUniformBuffer->buffer).commit();
    writeDescriptorSet(colorBuffer.getImageView(), depthBuffer.getImageView());
}

void PostProcessingPass::writeDescriptorSet(const VulkanImageView &colorView, const VulkanImageView &depthView) {
    // Fill the descriptor set
    perFrameDescriptorSet->startWriting()
            .writeImageSampler(1, colorBufferSampler->vk(), colorView.vk(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            .writeImageSampler(2, depthBufferSampler->vk(), depthView.vk(),
                               VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
            .commit();
}

void PostProcessingPass::draw() {
    assert(("No postprocessing configured",
            !(static_cast<ShaderConfig *>(uboContent.at(0))->cameraNear == 0.0f &&
              static_cast<ShaderConfig *>(uboContent.at(0))->cameraFar == 0.0f)));

    auto cmdBuf = context.getCurrentPrimaryCommandBuffer().vk();
    auto viewportWidth = context.getSwapChain().getWidth();
    auto viewportHeight = context.getSwapChain().getHeight();

    // Define render rendering to draw with
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->vk(); // the renderpass to use
    renderPassInfo.framebuffer = swapChainFrameBuffers[context.getCurrentSwapChainFrame()].vk(); // the attatchment
    renderPassInfo.renderArea.offset = {0, 0}; // size of the render area ...
    renderPassInfo.renderArea.extent = context.getSwapChain().getExtent(); // based on swap chain

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
    auto desc = perFrameDescriptorSet->vk();
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            postprocessingPipeline->getPipelineLayout(),
                            0, 1, &desc, 0, nullptr);

    VkBuffer vertexBuffers[]{quadBuffer->buffer};
    VkDeviceSize offsets[]{0};
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertexBuffers, offsets);
    // Draw a fullscreen quad and composite the final image
    vkCmdDraw(cmdBuf, 6, 1, 0, 0);

    vkCmdEndRenderPass(cmdBuf);
}

void PostProcessingPass::resizeAttachments(const VulkanImageBuffer &colorBuffer, const VulkanImageBuffer &depthBuffer) {
    swapChainFrameBuffers = createSwapChainFrameBuffers(context.getDevice(), context.getSwapChain(), *renderPass);
    writeDescriptorSet(colorBuffer.getImageView(), depthBuffer.getImageView());
}

void PostProcessingPass::updateConfiguration(const PostProcessingPass::PostProcessingConfiguration &configuration) {
    ShaderConfig *ubo = uboContent.at(0);

    ubo->cameraNear = configuration.camera.near;
    ubo->cameraFar = configuration.camera.far;
    // Copy that data to the uniform buffer
    context.getMemory().copyDataToBuffer(perFrameUniformBuffer->buffer, perFrameUniformBuffer->memory,
                                         uboContent.data(), uboContent.size());
}
