#include <imgui.h>
#include <src/renderer/passes/ImGuiRenderingPass.h>
#include "VulkanRenderer2D.h"
#include "src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "src/renderer/data/Mesh.h"
#include "src/renderer/data/ModelLoader.h"
#include "src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"


// ------------------------------------ Class Construction -------------------------------------------------------------

VulkanRenderer2D VulkanRenderer2D::Create(Window &window) {
    auto context = std::make_unique<VulkanContext>(window);

    auto spriteRenderingPass = SpriteRenderingPass::Create(*context, context->getSwapChain().getWidth(),
                                                           context->getSwapChain().getHeight());


    auto postProcessingPass = PostProcessingPass::Create(*context, spriteRenderingPass.getColorBuffer(),
                                                         spriteRenderingPass.getDepthBuffer());

    IMGUI_CHECKVERSION();
    ImGuiContext *imGuiContext = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    auto imGuiRenderingPass = ImGuiRenderingPass::Create(*context, window, context->getSwapChain().getWidth(),
                                                         context->getSwapChain().getHeight(), imGuiContext);

    return VulkanRenderer2D(std::move(context), std::move(spriteRenderingPass), std::move(postProcessingPass),
                            std::move(imGuiRenderingPass));
}

VulkanRenderer2D::VulkanRenderer2D(std::unique_ptr<VulkanContext> &&context, SpriteRenderingPass &&spriteRenderingPass,
                                   PostProcessingPass &&postProcessingPass, ImGuiRenderingPass &&imGuiRenderingPass)
        : context(std::move(context)), spriteRenderingPass(std::move(spriteRenderingPass)),
          postProcessingPass(std::move(postProcessingPass)), imGuiRenderingPass(std::move(imGuiRenderingPass)) {}


// ------------------------------------ Lifecycle methods --------------------------------------------------------------

void VulkanRenderer2D::setup() {
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

void VulkanRenderer2D::beginScene(const Camera &camera) {
    spriteRenderingPass.begin(camera);
}

void VulkanRenderer2D::endScene() {
    spriteRenderingPass.end();
    postProcessingPass.draw();
    imGuiRenderingPass.draw();
    context->getCurrentPrimaryCommandBuffer().end();
}

void VulkanRenderer2D::recreateSwapChain() {
    std::cout << "Recreating SwapChain" << std::endl;
    context->getDevice().waitIdle();

    // Update framebuffer attachments
    spriteRenderingPass.resizeAttachments(context->getSwapChain().getWidth(), context->getSwapChain().getHeight());
    imGuiRenderingPass.resizeAttachments(context->getSwapChain().getWidth(), context->getSwapChain().getHeight());
    postProcessingPass.resizeAttachments(spriteRenderingPass.getColorBuffer(), spriteRenderingPass.getDepthBuffer());
}

void VulkanRenderer2D::flush() {
    if (!context->flushCommands()) {
        // Display surface has changed -> update framebuffer attachments
        recreateSwapChain();
    }
}

void
VulkanRenderer2D::updatePostProcessingConfiguration(PostProcessingPass::PostProcessingConfiguration configuration) {
    postProcessingPass.updateConfiguration(configuration);
}

void VulkanRenderer2D::renderQuad(glm::mat4 modelMat, glm::vec4 color) {
    spriteRenderingPass.drawSprite(quadMesh, modelMat, color);
}