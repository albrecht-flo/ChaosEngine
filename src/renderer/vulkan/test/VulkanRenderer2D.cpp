#include "VulkanRenderer2D.h"
#include "src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptorSet.h"
#include "src/renderer/data/Mesh.h"
#include "src/renderer/data/ModelLoader.h"
#include "src/renderer/data/RenderObject.h"
#include "src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"

static std::vector<VulkanCommandBuffer>
createPrimaryCommandBuffers(const VulkanDevice &device, const VulkanCommandPool &commandPool, uint32_t swapChainSize) {
    std::vector<VulkanCommandBuffer> primaryCommandBuffers;
    primaryCommandBuffers.reserve(swapChainSize);
    for (uint32_t i = 0; i < swapChainSize; ++i) {
        primaryCommandBuffers.emplace_back(
                VulkanCommandBuffer::Create(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY));
    }

    return std::move(primaryCommandBuffers);
}

static VulkanImageBuffer
createDepthResources(const VulkanDevice &device, const VulkanMemory &vulkanMemory, VkExtent2D extent) {
    VkFormat depthFormat = VulkanImage::getDepthFormat(device);

    VkDeviceMemory depthImageMemory{};
    auto depthImage = VulkanImage::createDepthBufferImage(
            device, vulkanMemory, extent.width,
            extent.height,
            depthFormat, depthImageMemory);
    auto depthImageView = VulkanImageView::Create(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    return VulkanImageBuffer(device, std::move(depthImage), std::move(depthImageMemory), std::move(depthImageView));
}

static std::vector<VulkanFramebuffer>
createSwapChainFrameBuffers(const VulkanDevice &device, const VulkanSwapChain &swapChain,
                            const VulkanRenderPass &renderPass, const VulkanImageView &depthImage,
                            uint32_t maxFramesInFlight) {
    std::vector<VulkanFramebuffer> swapChainFramebuffers;
    swapChainFramebuffers.reserve(maxFramesInFlight);
    for (uint32_t i = 0; i < maxFramesInFlight; i++) {
        swapChainFramebuffers.emplace_back(
                renderPass.createFrameBuffer(
                        {swapChain.getImageViews()[i].vk(), depthImage.vk()},
                        swapChain.getExtent()
                ));
    }
    return std::move(swapChainFramebuffers);
}

// ------------------------------------ Class Construction -------------------------------------------------------------

VulkanRenderer2D VulkanRenderer2D::Create(Window &window) {
    auto context = std::make_unique<VulkanContext>(window);

    auto primaryCommandBuffers = createPrimaryCommandBuffers(context->getDevice(), context->getCommandPool(),
                                                             maxFramesInFlight);
    VulkanFrame frame = VulkanFrame::Create(window, *context, maxFramesInFlight);

    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(context->getDevice(), AttachmentType::Color).build());
    attachments.emplace_back(VulkanAttachmentBuilder(context->getDevice(), AttachmentType::Depth).build());
    VulkanRenderPass mainRenderPass = VulkanRenderPass::Create(context->getDevice(), attachments);

    VulkanImageBuffer depthBuffer = createDepthResources(context->getDevice(), context->getMemory(),
                                                         context->getSwapChain().getExtent());

    auto swapChainFrameBuffers = createSwapChainFrameBuffers(context->getDevice(), context->getSwapChain(),
                                                             mainRenderPass, depthBuffer.getImageView(),
                                                             context->getSwapChain().size());


    return VulkanRenderer2D(std::move(context), std::move(frame), std::move(primaryCommandBuffers),
                            std::move(mainRenderPass), std::move(depthBuffer), std::move(swapChainFrameBuffers));
}

VulkanRenderer2D::VulkanRenderer2D(std::unique_ptr<VulkanContext> &&context, VulkanFrame &&frame,
                                   std::vector<VulkanCommandBuffer> &&primaryCommandBuffers,
                                   VulkanRenderPass &&mainRenderPass, VulkanImageBuffer &&depthBuffer,
                                   std::vector<VulkanFramebuffer> &&swapChainFrameBuffers)
        : context(std::move(context)), frame(std::move(frame)), primaryCommandBuffers(std::move(primaryCommandBuffers)),
          mainRenderPass(std::move(mainRenderPass)), depthBuffer(std::move(depthBuffer)),
          swapChainFrameBuffers(std::move(swapChainFrameBuffers)) {}


// ------------------------------------ Lifecycle methods --------------------------------------------------------------

void VulkanRenderer2D::setup() {
    vertex_3P_3C_3N_2U = std::make_unique<VulkanVertexInput>(
            VertexAttributeBuilder(0, sizeof(Vertex), InputRate::Vertex)
                    .addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
                    .addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
                    .addAttribute(2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
                    .addAttribute(3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)).build());

    cameraDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(context->getDevice())
                    .addBinding(0, DescriptorType::UniformBuffer, ShaderStage::Vertex)
                    .build());
//    materialDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(
//            VulkanDescriptorSetLayoutBuilder(context->getDevice())
//                    .addBinding(0, DescriptorType::Texture, ShaderStage::Fragment)
//                    .build());

    VulkanPipelineLayout pipelineLayout = VulkanPipelineLayoutBuilder(context->getDevice())
            .addPushConstant(sizeof(glm::mat4), 0, ShaderStage::Vertex)
            .addPushConstant(sizeof(glm::vec4), sizeof(glm::mat4), ShaderStage::Fragment)
            .addDescriptorSet(*cameraDescriptorLayout) // set = 0
//            .addDescriptorSet(*materialDescriptorLayout) // set = 1
            .build();

    pipeline = std::make_unique<VulkanPipeline>(VulkanPipelineBuilder(context->getDevice(), mainRenderPass,
                                                                      std::move(pipelineLayout), *vertex_3P_3C_3N_2U,
                                                                      "2DSprite")
                                                        .setFragmentShader("2DStaticSprite")
                                                        .setTopology(Topology::TriangleList)
                                                        .setPolygonMode(PolygonMode::Fill)
                                                        .setCullFace(CullFace::CCLW)
                                                        .setDepthTestEnabled(true)
                                                        .setDepthCompare(CompareOp::Less)
                                                        .build());

    descriptorPool = std::make_unique<VulkanDescriptorPool>(VulkanDescriptorPoolBuilder(context->getDevice())
                                                                    .addDescriptor(cameraDescriptorLayout->getBinding(
                                                                            0).descriptorType, maxFramesInFlight)
//                                                                    .addDescriptor(materialDescriptorLayout->getBinding(
//                                                                            0).descriptorType, 1024)
                                                                    .setMaxSets(maxFramesInFlight + 1024)
                                                                    .build());


    perFrameUniformBuffers.resize(maxFramesInFlight);
    for (size_t i = 0; i < perFrameUniformBuffers.capacity(); i++) {
        perFrameUniformBuffers[i] = context->getMemory().createUniformBuffer(
                sizeof(CameraUbo),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                1, false);
    }
    uboContent = UniformBufferContent<CameraUbo>(1);

    perFrameDescriptorSets.reserve(maxFramesInFlight);
    for (size_t i = 0; i < perFrameDescriptorSets.capacity(); i++) {
        perFrameDescriptorSets.emplace_back(descriptorPool->allocate(*cameraDescriptorLayout));
        perFrameDescriptorSets[i].startWriting().writeBuffer(0, perFrameUniformBuffers[i].buffer).commit();
    }

    auto quad = ModelLoader::getQuad();
    VulkanBuffer vertexBuffer = context->getMemory().createInputBuffer(
            quad.vertices.size() * sizeof(quad.vertices[0]), quad.vertices.data(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VulkanBuffer indexBuffer = context->getMemory().createInputBuffer(
            quad.indices.size() * sizeof(quad.indices[0]), quad.indices.data(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    quadMesh = {.vertexBuffer=vertexBuffer, .indexBuffer=indexBuffer,
            .indexCount=static_cast<uint32_t>(quad.indices.size())};
}


void VulkanRenderer2D::join() {
    context->getDevice().waitIdle();
}

// ------------------------------------ Rendering methods --------------------------------------------------------------

void VulkanRenderer2D::updateUniformBuffer(glm::mat4 viewMat, glm::vec2 viewportDimensions) {
    float aspect = static_cast<float>(viewportDimensions.x) / viewportDimensions.y;

    CameraUbo *ubo = uboContent.at(currentFrame);
    ubo->view = viewMat;
    ubo->proj = glm::ortho(-3.0f*aspect, 3.0f*aspect, -3.0f, 3.0f, 0.1f, 100.0f);
    ubo->proj[1][1] *= -1;

    // Copy that data to the uniform buffer
    context->getMemory().copyDataToBuffer(perFrameUniformBuffers[currentFrame].buffer,
                                          perFrameUniformBuffers[currentFrame].memory,
                                          uboContent.data(), uboContent.size());
}

void VulkanRenderer2D::beginScene(const glm::mat4 &cameraTransform) {
    glm::uvec2 viewportDimensions(context->getSwapChain().getWidth(), context->getSwapChain().getHeight());
    updateUniformBuffer(cameraTransform, viewportDimensions);

    primaryCommandBuffers[currentFrame].begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
// Define render rendering to draw with
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mainRenderPass.vk(); // the renderpass to use
    renderPassInfo.framebuffer = swapChainFrameBuffers[currentSwapChainImage].vk(); // the attatchment
    renderPassInfo.renderArea.offset = {0, 0}; // size of the render area ...
    renderPassInfo.renderArea.extent = VkExtent2D{viewportDimensions.x, viewportDimensions.y}; // based on swap chain

    // Define the values used for VK_ATTACHMENT_LOAD_OP_CLEAR
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(primaryCommandBuffers[currentFrame].vk(), &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE); // use commands in primary cmdbuffer

    // Now vkCmd... can be written do define the draw call
    // Bind the pipline as a graphics pipeline
    vkCmdBindPipeline(primaryCommandBuffers[currentFrame].vk(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(viewportDimensions.x);
    viewport.height = static_cast<float>(viewportDimensions.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(primaryCommandBuffers[currentFrame].vk(), 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {context->getSwapChain().getWidth(), context->getSwapChain().getHeight()};
    vkCmdSetScissor(primaryCommandBuffers[currentFrame].vk(), 0, 1, &scissor);

    // Bind the descriptor set to the pipeline
    auto cameraDescriptor = perFrameDescriptorSets[currentFrame].vk();
    vkCmdBindDescriptorSets(primaryCommandBuffers[currentFrame].vk(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline->getPipelineLayout(),
                            0, 1, &cameraDescriptor, 0, nullptr);

}

void VulkanRenderer2D::endScene() {
    vkCmdEndRenderPass(primaryCommandBuffers[currentFrame].vk());
    primaryCommandBuffers[currentFrame].end();
}

void VulkanRenderer2D::recreateSwapChain() {
    std::cout << "Recreating SwapChain" << std::endl;
    context->getDevice().waitIdle();
    // TODO: Recreate swap chain associated resources
    context->recreateSwapChain();

    depthBuffer = createDepthResources(context->getDevice(), context->getMemory(), context->getSwapChain().getExtent());
    // Recreate the frame buffers pointing to the swap chain images
    swapChainFrameBuffers = createSwapChainFrameBuffers(context->getDevice(), context->getSwapChain(),
                                                        mainRenderPass, depthBuffer.getImageView(),
                                                        context->getSwapChain().size());

    // Update framebuffer bindings as textures in post processing
}

void VulkanRenderer2D::flush() {
    if (!frame.render(currentFrame, primaryCommandBuffers[currentFrame])) {
        recreateSwapChain();
    }
    currentFrame = (currentFrame < maxFramesInFlight - 1) ? currentFrame + 1 : 0;
    currentSwapChainImage = (currentSwapChainImage < context->getSwapChain().size() - 1) ? currentSwapChainImage + 1
                                                                                         : 0;
}

void VulkanRenderer2D::renderQuad(glm::mat4 modelMat, glm::vec4 color) {
    // Set model matrix via push constant
    vkCmdPushConstants(primaryCommandBuffers[currentFrame].vk(), pipeline->getPipelineLayout(),
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(modelMat), &modelMat);
    // Set model matrix via push constant
    vkCmdPushConstants(primaryCommandBuffers[currentFrame].vk(), pipeline->getPipelineLayout(),
                       VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(modelMat), sizeof(color), &color);

    VkBuffer vertexBuffers[]{quadMesh.vertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(primaryCommandBuffers[currentFrame].vk(), 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(primaryCommandBuffers[currentFrame].vk(), quadMesh.indexBuffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);
    // Draw a fullscreen quad and composite the final image
    vkCmdDrawIndexed(primaryCommandBuffers[currentFrame].vk(), quadMesh.indexCount, 1, 0, 0, 0);
}