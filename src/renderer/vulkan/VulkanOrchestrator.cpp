#include "VulkanOrchestrator.h"
#include "command/VulkanCommandBuffer.h"

static std::vector<VulkanCommandBuffer>
createPrimaryCommandBuffers(const VulkanContext &context, uint32_t swapChainSize) {
    std::vector<VulkanCommandBuffer> primaryCommandBuffers;
    primaryCommandBuffers.reserve(swapChainSize);
    for (int i = 0; i < swapChainSize; ++i) {
        primaryCommandBuffers.emplace_back(
                VulkanCommandBuffer::Create(context.getDevice(), context.getCommandPool(),
                                            VK_COMMAND_BUFFER_LEVEL_PRIMARY));
    }

    return std::move(primaryCommandBuffers);
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanOrchestrator VulkanOrchestrator::Create(const Window &window) {
    VulkanContext context = VulkanContext::Create(window);

    VulkanSwapChain swapChain = VulkanSwapChain::Create(window, context.getDevice(), context.getSurface());

    auto primaryCommandBuffers = createPrimaryCommandBuffers(context, swapChain.size());

    // TODO: Implement move = operator to handle recreation
    // vulkanDataManager.init();
    // solidRenderPass.init();
    // transparentRenderPass.init();
    // guiRenderPass.init();
    // postProcessingRenderPass.init();
    return VulkanOrchestrator(std::move(context), std::move(swapChain), std::move(primaryCommandBuffers));
}

VulkanOrchestrator::VulkanOrchestrator(VulkanContext &&context, VulkanSwapChain &&swapChain,
                                       std::vector<VulkanCommandBuffer> &&primaryCommandBuffers)
        : context(std::move(context)),
          swapChain(std::move(swapChain)),
          primaryCommandBuffers(std::move(primaryCommandBuffers)) {
    // vulkanDataManager(vulkanContext),
}

void VulkanOrchestrator::join() {

}

void VulkanOrchestrator::beginScene() {

}

void VulkanOrchestrator::useShader(RendererAPI::ShaderRef shaderRef) {

}

void VulkanOrchestrator::endScene() {

}

void VulkanOrchestrator::flush() {

}

void VulkanOrchestrator::renderObject(RendererAPI::MeshRef meshRef, RendererAPI::MaterialRef materialRef,
                                      glm::mat4 modelMat) {

}

RendererAPI::ShaderRef VulkanOrchestrator::loadShader() {
    return RendererAPI::ShaderRef();
}

RendererAPI::MeshRef VulkanOrchestrator::loadMesh() {
    return RendererAPI::MeshRef();
}

RendererAPI::MaterialRef VulkanOrchestrator::loadMaterial() {
    return RendererAPI::MaterialRef();
}




