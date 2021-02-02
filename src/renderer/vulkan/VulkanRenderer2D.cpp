#include <src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h>
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

    VulkanDataManager pipelineManager{};

    return VulkanRenderer2D(std::move(context), std::move(frame), std::move(swapChainFrameBuffers),
                            std::move(mainRenderPass), std::move(depthBuffer), std::move(pipelineManager));
}

VulkanRenderer2D::VulkanRenderer2D(VulkanContext &&context, VulkanFrame &&frame,
                                   std::vector<VulkanFramebuffer> &&swapChainFrameBuffers,
                                   VulkanRenderPass &&mainRenderPass, VulkanImageBuffer &&depthBuffer,
                                   VulkanDataManager &&pipelineManager)
        : context(std::move(context)), frame(std::move(frame)), swapChainFrameBuffers(std::move(swapChainFrameBuffers)),
          mainRenderPass(std::move(mainRenderPass)), depthBuffer(std::move(depthBuffer)),
          pipelineManager(std::move(pipelineManager)) {}

// ------------------------------------ Rendering methods --------------------------------------------------------------

void VulkanRenderer2D::join() {
    context.getDevice().waitIdle();
}

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

    // Recreate render passes
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

RendererAPI::ShaderRef VulkanRenderer2D::loadShader() {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv;
    };
    VulkanVertexInput Vertex_3P_3C_3N_2U = VertexAttributeBuilder(0, sizeof(Vertex), InputRate::Vertex)
            .addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
            .addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
            .addAttribute(2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
            .addAttribute(3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)).build();

    VulkanPipeline pipeline = VulkanPipelineBuilder(context.getDevice(), mainRenderPass, "base", Vertex_3P_3C_3N_2U)
            .setTopology(Topology::TriangleList)
            .setPolygonMode(PolygonMode::Fill)
            .setCullFace(CullFace::CCLW)
            .setDepthTestEnabled(true)
            .setDepthCompare(CompareOp::Less)
            .build();

    pipelineManager.addNewPipeline(std::move(pipeline));
    return RendererAPI::ShaderRef();
}

RendererAPI::MeshRef VulkanRenderer2D::loadMesh() {
    return RendererAPI::MeshRef();
}

RendererAPI::MaterialRef VulkanRenderer2D::loadMaterial() {
    return RendererAPI::MaterialRef();
}

