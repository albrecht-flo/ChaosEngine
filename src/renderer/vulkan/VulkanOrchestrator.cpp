#include "VulkanOrchestrator.h"

VulkanOrchestrator::VulkanOrchestrator(const Window &window, VulkanContext &&context, VulkanSwapChain &&swapChain)
        : context(std::move(context)),
          memory(context.getDevice(), context.getCommandPool().vk()),
          swapChain(std::move(swapChain)) {
    // vulkanDataManager(vulkanContext),
}

VulkanOrchestrator VulkanOrchestrator::Create(const Window &window) {
    VulkanContext context = VulkanContext::Create(window);

    VulkanSwapChain swapChain = VulkanSwapChain::Create(window, context.getDevice(), context.getSurface());

    // vulkanDataManager.init();
    // solidRenderPass.init();
    // transparentRenderPass.init();
    // guiRenderPass.init();
    // postProcessingRenderPass.init();
    return VulkanOrchestrator(window, std::move(context), std::move(swapChain));
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




