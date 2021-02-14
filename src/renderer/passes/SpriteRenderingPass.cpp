#include "SpriteRenderingPass.h"
#include "src/renderer/data/Mesh.h"
#include "src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorSetLayoutBuilder.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorPoolBuilder.h"
#include "src/renderer/vulkan/pipeline/VulkanPipelineLayoutBuilder.h"
#include "src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"

static VulkanImageBuffer
createImageBuffer(const VulkanDevice &device, const VulkanMemory &vulkanMemory, uint32_t width, uint32_t height) {
    VkDeviceMemory imageMemory{};
    auto depthImage = VulkanImage::createDepthBufferImage(
            device, vulkanMemory, width, height, VK_FORMAT_R8G8B8A8_UNORM, imageMemory);
    auto depthImageView = VulkanImageView::Create(device, depthImage, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_ASPECT_DEPTH_BIT);

    return VulkanImageBuffer(device, std::move(depthImage), std::move(imageMemory), std::move(depthImageView),
                             width, height);
}

static VulkanImageBuffer
createDepthResources(const VulkanDevice &device, const VulkanMemory &vulkanMemory, uint32_t width, uint32_t height) {
    VkFormat depthFormat = VulkanImage::getDepthFormat(device);

    VkDeviceMemory depthImageMemory{};
    auto depthImage = VulkanImage::createDepthBufferImage(
            device, vulkanMemory, width, height, depthFormat, depthImageMemory);
    auto depthImageView = VulkanImageView::Create(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    return VulkanImageBuffer(device, std::move(depthImage), std::move(depthImageMemory), std::move(depthImageView),
                             width, height);
}


static std::vector<VulkanFramebuffer>
createSwapChainFrameBuffers(const VulkanDevice &device, const VulkanSwapChain &swapChain,
                            const VulkanRenderPass &renderPass, const VulkanImageBuffer &depthBuffer) {
    std::vector<VulkanFramebuffer> swapChainFramebuffers;
    swapChainFramebuffers.reserve(swapChain.size());
    for (uint32_t i = 0; i < swapChain.size(); i++) {
        swapChainFramebuffers.emplace_back(
                renderPass.createFrameBuffer(
                        {swapChain.getImageViews()[i].vk(), depthBuffer.getImageView().vk()},
                        swapChain.getExtent()
                ));
    }
    return std::move(swapChainFramebuffers);
}

// ------------------------------------ Class Members ------------------------------------------------------------------

SpriteRenderingPass
SpriteRenderingPass::Create(const VulkanContext &context, uint32_t width, uint32_t height, bool renderToSwapChain) {
    auto ret = SpriteRenderingPass(context, renderToSwapChain);
    ret.init(width, height);
    return std::move(ret);
}

SpriteRenderingPass::SpriteRenderingPass(SpriteRenderingPass &&o) noexcept:
        context(o.context), opaquePass(std::move(o.opaquePass)), renderToSwapChain(o.renderToSwapChain),
        colorBuffer(std::move(o.colorBuffer)), depthBuffer(std::move(o.depthBuffer)),
        framebuffer(std::move(o.framebuffer)),
        swapChainFrameBuffers(std::move(o.swapChainFrameBuffers)),
        descriptorPool(std::move(o.descriptorPool)),
        cameraDescriptorLayout(std::move(o.cameraDescriptorLayout)),
        materialDescriptorLayout(std::move(o.materialDescriptorLayout)),
        pipeline(std::move(o.pipeline)),
        perFrameDescriptorSets(std::move(o.perFrameDescriptorSets)),
        perFrameUniformBuffers(std::move(o.perFrameUniformBuffers)),
        uboContent(std::move(o.uboContent)) {}

void SpriteRenderingPass::createAttachments(uint32_t width, uint32_t height) {
    colorBuffer = std::make_unique<VulkanImageBuffer>(
            createImageBuffer(context.getDevice(), context.getMemory(), width, height));
    depthBuffer = std::make_unique<VulkanImageBuffer>(
            createDepthResources(context.getDevice(), context.getMemory(), width, height));
    if (!renderToSwapChain) {
        framebuffer = std::make_unique<VulkanFramebuffer>(opaquePass->createFrameBuffer(
                {colorBuffer->getImageView().vk(), depthBuffer->getImageView().vk()},
                {colorBuffer->getWidth(), colorBuffer->getHeight()}));
    } else {
        swapChainFrameBuffers = createSwapChainFrameBuffers(context.getDevice(), context.getSwapChain(), *opaquePass,
                                                            *depthBuffer);
    }
}

void SpriteRenderingPass::createStandardPipeline() {
    auto vertex_3P_3C_3N_2U = std::make_unique<VulkanVertexInput>(
            VertexAttributeBuilder(0, sizeof(Vertex), InputRate::Vertex)
                    .addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
                    .addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
                    .addAttribute(2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
                    .addAttribute(3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)).build());

    cameraDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(context.getDevice())
                    .addBinding(0, DescriptorType::UniformBuffer, ShaderStage::Vertex)
                    .build());
//    materialDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(
//            VulkanDescriptorSetLayoutBuilder(context.getDevice())
//                    .addBinding(0, DescriptorType::Texture, ShaderStage::Fragment)
//                    .build());

    VulkanPipelineLayout pipelineLayout = VulkanPipelineLayoutBuilder(context.getDevice())
            .addPushConstant(sizeof(glm::mat4), 0, ShaderStage::Vertex)
            .addPushConstant(sizeof(glm::vec4), sizeof(glm::mat4), ShaderStage::Fragment)
            .addDescriptorSet(*cameraDescriptorLayout) // set = 0
//            .addDescriptorSet(*materialDescriptorLayout) // set = 1
            .build();

    pipeline = std::make_unique<VulkanPipeline>(VulkanPipelineBuilder(context.getDevice(), *opaquePass,
                                                                      std::move(pipelineLayout), *vertex_3P_3C_3N_2U,
                                                                      "2DSprite")
                                                        .setFragmentShader("2DStaticSprite")
                                                        .setTopology(Topology::TriangleList)
                                                        .setPolygonMode(PolygonMode::Fill)
                                                        .setCullFace(CullFace::CCLW)
                                                        .setDepthTestEnabled(true)
                                                        .setDepthCompare(CompareOp::Less)
                                                        .build());

    descriptorPool = std::make_unique<VulkanDescriptorPool>(VulkanDescriptorPoolBuilder(context.getDevice())
                                                                    .addDescriptor(cameraDescriptorLayout->getBinding(
                                                                            0).descriptorType,
                                                                                   VulkanContext::maxFramesInFlight)
//                                                                    .addDescriptor(materialDescriptorLayout->getBinding(
//                                                                            0).descriptorType, 1024)
                                                                    .setMaxSets(VulkanContext::maxFramesInFlight + 1024)
                                                                    .build());


    perFrameUniformBuffers.resize(VulkanContext::maxFramesInFlight);
    for (size_t i = 0; i < perFrameUniformBuffers.capacity(); i++) {
        perFrameUniformBuffers[i] = context.getMemory().createUniformBuffer(
                sizeof(CameraUbo),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                1, false);
    }
    uboContent = UniformBufferContent<CameraUbo>(1);

    perFrameDescriptorSets.reserve(VulkanContext::maxFramesInFlight);
    for (size_t i = 0; i < perFrameDescriptorSets.capacity(); i++) {
        perFrameDescriptorSets.emplace_back(descriptorPool->allocate(*cameraDescriptorLayout));
        perFrameDescriptorSets[i].startWriting().writeBuffer(0, perFrameUniformBuffers[i].buffer).commit();
    }

}

void SpriteRenderingPass::init(uint32_t width, uint32_t height) {
    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(context.getDevice(), AttachmentType::Color)
                                     .layoutInitFinal(VK_IMAGE_LAYOUT_UNDEFINED,
                                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                                     .build());
    attachments.emplace_back(VulkanAttachmentBuilder(context.getDevice(), AttachmentType::Depth).build());
    opaquePass = std::make_unique<VulkanRenderPass>(VulkanRenderPass::Create(context.getDevice(), attachments));

    createAttachments(width, height);

    createStandardPipeline();
}

void SpriteRenderingPass::updateUniformBuffer(const Camera &camera, const glm::vec2 &viewportDimensions) {

    CameraUbo *ubo = uboContent.at(context.getCurrentFrame());
    ubo->view = camera.view;
    if (viewportDimensions.x > viewportDimensions.y) {
        float aspect = static_cast<float>(viewportDimensions.x) / viewportDimensions.y;
        ubo->proj = glm::ortho(-camera.fieldOfView * aspect, camera.fieldOfView * aspect, -camera.fieldOfView,
                               camera.fieldOfView, camera.near, camera.far);
    } else {
        float aspect = static_cast<float>(viewportDimensions.y) / viewportDimensions.x;
        ubo->proj = glm::ortho(-3.0f, 3.0f, -3.0f * aspect, 3.0f * aspect, 0.1f, 100.0f);
    }
    ubo->proj[1][1] *= -1; // GLM uses OpenGL projection -> Y Coordinate needs to be fliped

    // Copy that data to the uniform buffer
    context.getMemory().copyDataToBuffer(perFrameUniformBuffers[context.getCurrentFrame()].buffer,
                                         perFrameUniformBuffers[context.getCurrentFrame()].memory,
                                         uboContent.data(), uboContent.size());
}

void SpriteRenderingPass::begin(const Camera &camera) {
    glm::uvec2 viewportDimensions(context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
    updateUniformBuffer(camera, viewportDimensions);

    auto &commandBuffer = context.getCurrentPrimaryCommandBuffer();
    commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    // Define render rendering to draw with
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = opaquePass->vk(); // the renderpass to use
    if (!renderToSwapChain)
        renderPassInfo.framebuffer = framebuffer->vk();
    else
        renderPassInfo.framebuffer = swapChainFrameBuffers[context.getCurrentSwapChainFrame()].vk(); // the attatchment
    renderPassInfo.renderArea.offset = {0, 0}; // size of the render area ...
    renderPassInfo.renderArea.extent = VkExtent2D{viewportDimensions.x, viewportDimensions.y}; // based on swap chain

    // Define the values used for VK_ATTACHMENT_LOAD_OP_CLEAR
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer.vk(), &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE); // use commands in primary cmdbuffer

    // Now vkCmd... can be written do define the draw call
    // Bind the pipline as a graphics pipeline
    vkCmdBindPipeline(commandBuffer.vk(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(viewportDimensions.x);
    viewport.height = static_cast<float>(viewportDimensions.y);
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
SpriteRenderingPass::drawSprite(const RenderMesh &renderObject, const glm::mat4 &modelMat, const glm::vec4 &color) {
    auto &commandBuffer = context.getCurrentPrimaryCommandBuffer();
    // Set model matrix via push constant
    vkCmdPushConstants(commandBuffer.vk(), pipeline->getPipelineLayout(),
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(modelMat), &modelMat);
    // Set model matrix via push constant
    vkCmdPushConstants(commandBuffer.vk(), pipeline->getPipelineLayout(),
                       VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(modelMat), sizeof(color), &color);

    VkBuffer vertexBuffers[]{renderObject.vertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer.vk(), 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer.vk(), renderObject.indexBuffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);
    // Draw a fullscreen quad and composite the final image
    vkCmdDrawIndexed(commandBuffer.vk(), renderObject.indexCount, 1, 0, 0, 0);
}
