#include <src/renderer/vulkan/rendering/VulkanFrame.h>
#include <src/renderer/vulkan/image/VulkanFramebuffer.h>
#include <src/renderer/vulkan/rendering/VulkanRenderPass.h>
#include "VulkanRenderer2D.h"

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

static std::vector<VulkanFramebuffer>
createSwapChainFrameBuffers(const VulkanDevice &device, const VulkanSwapChain &swapChain,
                            const VulkanRenderPass &renderPass,
                            uint32_t maxFramesInFlight) {
    std::vector<VulkanFramebuffer> swapChainFramebuffers;
    swapChainFramebuffers.reserve(maxFramesInFlight);
    for (uint32_t i = 0; i < maxFramesInFlight; i++) {
        swapChainFramebuffers.emplace_back(VulkanFramebuffer::createFramebuffer(
                device,
                {swapChain.getImageViews()[i]},
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

    // TODO: auto swapChainFrameBuffers = createSwapChainFrameBuffers(context.getDevice(), context.getSwapChain(), postProcessingPass, maxFramesInFlight);
    return VulkanRenderer2D(std::move(context), std::move(frame) /*, std::move(swapChainFrameBuffers)*/);
}

VulkanRenderer2D::VulkanRenderer2D(VulkanContext &&context, VulkanFrame &&frame)
        : context(std::move(context)), frame(std::move(frame)) {}

VulkanRenderer2D::VulkanRenderer2D(VulkanRenderer2D &&o) noexcept
        : context(std::move(o.context)), frame(std::move(o.frame)) {}

// ------------------------------------ Rendering methods --------------------------------------------------------------

void VulkanRenderer2D::join() {

}

void VulkanRenderer2D::beginScene() {

}

void VulkanRenderer2D::useShader(RendererAPI::ShaderRef shaderRef) {

}

void VulkanRenderer2D::endScene() {

}

void VulkanRenderer2D::flush() {
    if (!frame.render(currentFrame, primaryCommandBuffers[currentFrame])) {
        // TODO: Recreate swap chain associated resources
        context.recreateSwapChain();
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

