#include "VulkanRenderer2D.h"

#include "src/renderer/vulkan/image/VulkanImage.h"

static std::vector<VulkanCommandBuffer>
createPrimaryCommandBuffers(const VulkanDevice &device, const VulkanCommandPool &commandPool, uint32_t swapChainSize) {
    std::vector<VulkanCommandBuffer> primaryCommandBuffers;
    primaryCommandBuffers.reserve(swapChainSize);
    for (int i = 0; i < swapChainSize; ++i) {
        primaryCommandBuffers.emplace_back(
                VulkanCommandBuffer::Create(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY));
    }

    return std::move(primaryCommandBuffers);
}

static std::tuple<VkImage, VkDeviceMemory, VulkanImageView>
createDepthResources(const VulkanDevice &device, const VulkanMemory &vulkanMemory, VkExtent2D extent) {
    VkFormat depthFormat = VulkanImage::getDepthFormat(device);

    VkDeviceMemory depthImageMemory{};
    auto depthImage = VulkanImage::createDepthBufferImage(
            device, vulkanMemory, extent.width,
            extent.height,
            depthFormat, depthImageMemory);
    auto depthImageView = VulkanImageView::Create(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    return std::make_tuple(depthImage, depthImageMemory, std::move(depthImageView));
}

static std::vector<VulkanFramebuffer>
createSwapChainFrameBuffers(const VulkanDevice &device, const VulkanSwapChain &swapChain,
                            const VulkanRenderPass &renderPass, const VulkanImageView &depthImage,
                            uint32_t maxFramesInFlight) {
    std::vector<VulkanFramebuffer> swapChainFramebuffers;
    swapChainFramebuffers.reserve(maxFramesInFlight);
    for (uint32_t i = 0; i < maxFramesInFlight; i++) {
        swapChainFramebuffers.emplace_back(VulkanFramebuffer::createFramebuffer(
                device,
                {swapChain.getImageViews()[i].vk(), depthImage.vk()},
                renderPass.vk(),
                swapChain.getExtent().width, swapChain.getExtent().height
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

    auto[depthImage, depthImageMemory, depthImageView] = createDepthResources(context.getDevice(), context.getMemory(),
                                                                              context.getSwapChain().getExtent());

    auto swapChainFrameBuffers = createSwapChainFrameBuffers(context.getDevice(), context.getSwapChain(),
                                                             mainRenderPass, depthImageView, maxFramesInFlight);

    return VulkanRenderer2D(std::move(context), std::move(frame), std::move(swapChainFrameBuffers),
                            std::move(mainRenderPass), depthImage, depthImageMemory, std::move(depthImageView));
}

VulkanRenderer2D::VulkanRenderer2D(VulkanContext &&context, VulkanFrame &&frame,
                                   std::vector<VulkanFramebuffer> &&swapChainFrameBuffers,
                                   VulkanRenderPass &&mainRenderPass, VkImage depthImage,
                                   VkDeviceMemory depthImageMemory, VulkanImageView &&depthImageView)
        : context(std::move(context)), frame(std::move(frame)), swapChainFrameBuffers(std::move(swapChainFrameBuffers)),
          mainRenderPass(std::move(mainRenderPass)), depthImage(depthImage), depthImageMemory(depthImageMemory),
          depthImageView(std::move(depthImageView)) {}

// ------------------------------------ Rendering methods --------------------------------------------------------------

void VulkanRenderer2D::join() {

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

    // TODO: refactor vulkan images
    VulkanImage::destroy(context.getDevice(), depthImage, depthImageMemory);
    auto[mDepthImage, mDepthImageMemory, mDepthImageView] = createDepthResources(context.getDevice(),
                                                                                 context.getMemory(),
                                                                                 context.getSwapChain().getExtent());
    depthImage = mDepthImage;
    depthImageMemory = mDepthImageMemory;
    depthImageView = std::move(mDepthImageView);
    // Recreate the frame buffers pointing to the swap chain images
    swapChainFrameBuffers = createSwapChainFrameBuffers(context.getDevice(), context.getSwapChain(),
                                                        mainRenderPass, depthImageView, maxFramesInFlight);

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
    return RendererAPI::ShaderRef();
}

RendererAPI::MeshRef VulkanRenderer2D::loadMesh() {
    return RendererAPI::MeshRef();
}

RendererAPI::MaterialRef VulkanRenderer2D::loadMaterial() {
    return RendererAPI::MaterialRef();
}

