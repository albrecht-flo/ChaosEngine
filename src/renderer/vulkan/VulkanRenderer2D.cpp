#include <src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h>
#include <src/renderer/vulkan/pipeline/VulkanDescriptorSetLayout.h>
#include <src/renderer/vulkan/pipeline/VulkanDescriptorSet.h>
#include <src/renderer/data/Mesh.h>
#include "VulkanRenderer2D.h"
#include "src/renderer/vulkan/pipeline/VulkanVertexInput.h"
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
    VulkanContext context = VulkanContext::Create(window);

    auto primaryCommandBuffers = createPrimaryCommandBuffers(context.getDevice(), context.getCommandPool(),
                                                             maxFramesInFlight);
    VulkanFrame frame = VulkanFrame::Create(window, context, maxFramesInFlight);

    std::vector<VulkanAttachmentDescription> attachments;
    attachments.emplace_back(VulkanAttachmentBuilder(context.getDevice(), AttachmentType::Color).build());
    attachments.emplace_back(VulkanAttachmentBuilder(context.getDevice(), AttachmentType::Depth).build());
    VulkanRenderPass mainRenderPass = VulkanRenderPass::Create(context.getDevice(), attachments);

    VulkanImageBuffer depthBuffer = createDepthResources(context.getDevice(), context.getMemory(),
                                                         context.getSwapChain().getExtent());

    auto swapChainFrameBuffers = createSwapChainFrameBuffers(context.getDevice(), context.getSwapChain(),
                                                             mainRenderPass, depthBuffer.getImageView(),
                                                             maxFramesInFlight);


    return VulkanRenderer2D(std::move(context), std::move(frame), std::move(swapChainFrameBuffers),
                            std::move(mainRenderPass), std::move(depthBuffer));
}

VulkanRenderer2D::VulkanRenderer2D(VulkanContext &&context, VulkanFrame &&frame,
                                   std::vector<VulkanFramebuffer> &&swapChainFrameBuffers,
                                   VulkanRenderPass &&mainRenderPass, VulkanImageBuffer &&depthBuffer)
        : context(std::move(context)), frame(std::move(frame)), swapChainFrameBuffers(std::move(swapChainFrameBuffers)),
          mainRenderPass(std::move(mainRenderPass)), depthBuffer(std::move(depthBuffer)),
          pipelineManager{}, vulkanMemory(context.getDevice(), context.getCommandPool()) {}


// ------------------------------------ Lifecycle methods --------------------------------------------------------------

void VulkanRenderer2D::setup() {
    vertex_3P_3C_3N_2U = std::make_unique<VulkanVertexInput>(
            VertexAttributeBuilder(0, sizeof(Vertex), InputRate::Vertex)
                    .addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
                    .addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
                    .addAttribute(2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
                    .addAttribute(3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)).build());

    cameraDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(context.getDevice())
                    .addBinding(0, DescriptorType::UniformBuffer, ShaderStage::Vertex, 1)
                    .build());
    materialDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(
            VulkanDescriptorSetLayoutBuilder(context.getDevice())
                    .addBinding(0, DescriptorType::Texture, ShaderStage::Fragment)
                    .build());

    VulkanPipelineLayout pipelineLayout = VulkanPipelineLayoutBuilder(context.getDevice())
            .addPushConstant(sizeof(glm::mat4), 0, ShaderStage::Vertex)
            .addDescriptorSet(*cameraDescriptorLayout) // set = 0
            .addDescriptorSet(*materialDescriptorLayout) // set = 1
            .build();

    pipeline = std::make_unique<VulkanPipeline>(VulkanPipelineBuilder(context.getDevice(), mainRenderPass,
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
                                                                            0).descriptorType, maxFramesInFlight)
                                                                    .addDescriptor(materialDescriptorLayout->getBinding(
                                                                            0).descriptorType, 1024)
                                                                    .setMaxSets(maxFramesInFlight * 1024)
                                                                    .build());

    perFrameDescriptorSets.reserve(maxFramesInFlight);
    perFrameUniformBuffers.reserve(perFrameDescriptorSets.size());

    for (size_t i = 0; i < perFrameDescriptorSets.size(); i++) {
        perFrameUniformBuffers.emplace_back(vulkanMemory.createUniformBuffer(
                sizeof(CameraUbo), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1,
                false
        ));
        perFrameDescriptorSets.emplace_back(descriptorPool->allocate(*cameraDescriptorLayout));
//        perFrameDescriptorSets[i].startWriting().writeBuffer(0, perFrameUniformBuffers[i].buffer).commit();
    }
}


void VulkanRenderer2D::join() {
    context.getDevice().waitIdle();
}

// ------------------------------------ Rendering methods --------------------------------------------------------------

void VulkanRenderer2D::beginScene() {

}

void VulkanRenderer2D::useShader(RendererAPI::ShaderRef shaderRef) {

}

void VulkanRenderer2D::endScene() {

}

void VulkanRenderer2D::recreateSwapChain() {
    context.getDevice().waitIdle();
    // TODO: Recreate swap chain associated resources
    context.recreateSwapChain();

    depthBuffer = createDepthResources(context.getDevice(), context.getMemory(), context.getSwapChain().getExtent());
    // Recreate the frame buffers pointing to the swap chain images
    swapChainFrameBuffers = createSwapChainFrameBuffers(context.getDevice(), context.getSwapChain(),
                                                        mainRenderPass, depthBuffer.getImageView(), maxFramesInFlight);

    // Update framebuffer bindings as textures in post processing
}

void VulkanRenderer2D::flush() {
    if (!frame.render(currentFrame, primaryCommandBuffers[currentFrame])) {
        recreateSwapChain();
    }
    currentFrame = (currentFrame < maxFramesInFlight - 1) ? currentFrame + 1 : 0;
}

void
VulkanRenderer2D::renderObject(RendererAPI::MeshRef meshRef, RendererAPI::MaterialRef materialRef, glm::mat4 modelMat) {

}

// ------------------------------------ Data management methods --------------------------------------------------------

RendererAPI::MeshRef VulkanRenderer2D::loadMesh() {
    return RendererAPI::MeshRef();
}

RendererAPI::MaterialRef VulkanRenderer2D::loadMaterial() {
    return RendererAPI::MaterialRef();
}

