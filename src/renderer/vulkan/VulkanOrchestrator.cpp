#include "VulkanOrchestrator.h"

VulkanOrchestrator::VulkanOrchestrator(Window &window, VulkanContext &&context) : context(std::move(context)) {
    // vulkanDataManager(vulkanContext),
}

VulkanOrchestrator VulkanOrchestrator::Create(Window &window) {
    VulkanContext contest = VulkanContext::Create(window);

    // vulkanDataManager.init();
    // solidRenderPass.init();
    // transparentRenderPass.init();
    // guiRenderPass.init();
    // postProcessingRenderPass.init();
    return VulkanOrchestrator(window, std::move(context));
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




