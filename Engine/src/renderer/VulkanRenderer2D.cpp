#include "VulkanRenderer2D.h"

#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "Engine/src/renderer/data/Mesh.h"
#include "Engine/src/renderer/data/ModelLoader.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"
#include "Engine/src/renderer/api/Material.h"

#include <imgui.h>

// ------------------------------------ Class Construction -------------------------------------------------------------

std::unique_ptr<VulkanRenderer2D> VulkanRenderer2D::Create(Renderer::GraphicsContext &graphicsContext) {
    auto &context = dynamic_cast<VulkanContext &>(graphicsContext);
    auto spriteRenderingPass = SpriteRenderingPass::Create(context, context.getSwapChain().getWidth(),
                                                           context.getSwapChain().getHeight());


    auto postProcessingPass = PostProcessingPass::Create(context, spriteRenderingPass.getFramebuffer(), false,
                                                         context.getSwapChain().getWidth(),
                                                         context.getSwapChain().getHeight());

    IMGUI_CHECKVERSION();
    ImGuiContext *imGuiContext = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable new Viewport feature
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable new Docking feature
    ImGui::StyleColorsDark();
    auto imGuiRenderingPass = ImGuiRenderingPass::Create(context, context.getWindow(),
                                                         context.getSwapChain().getWidth(),
                                                         context.getSwapChain().getHeight(), imGuiContext);

    return std::unique_ptr<VulkanRenderer2D>(
            new VulkanRenderer2D(context, std::move(spriteRenderingPass), std::move(postProcessingPass),
                                 std::move(imGuiRenderingPass)));
}

VulkanRenderer2D::VulkanRenderer2D(VulkanContext &context, SpriteRenderingPass &&spriteRenderingPass,
                                   PostProcessingPass &&postProcessingPass, ImGuiRenderingPass &&imGuiRenderingPass)
        : context(context), spriteRenderingPass(std::move(spriteRenderingPass)),
          postProcessingPass(std::move(postProcessingPass)), imGuiRenderingPass(std::move(imGuiRenderingPass)) {}


// ------------------------------------ Lifecycle methods --------------------------------------------------------------

void VulkanRenderer2D::setup() {
    auto quad = ModelLoader::getQuad();
    VulkanBuffer vertexBuffer = context.getMemory().createInputBuffer(
            quad.vertices.size() * sizeof(quad.vertices[0]), reinterpret_cast<const char *>(quad.vertices.data()),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VulkanBuffer indexBuffer = context.getMemory().createInputBuffer(
            quad.indices.size() * sizeof(quad.indices[0]), reinterpret_cast<const char *>(quad.indices.data()),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    quadMesh = {.vertexBuffer=vertexBuffer, .indexBuffer=indexBuffer,
            .indexCount=static_cast<uint32_t>(quad.indices.size())};
}


void VulkanRenderer2D::join() {
    context.getDevice().waitIdle();
}

// ------------------------------------ Rendering methods --------------------------------------------------------------

void VulkanRenderer2D::beginScene(const glm::mat4 &viewMat, const CameraComponent &camera) {
    spriteRenderingPass.begin(viewMat, camera);
    postProcessingPass.updateConfiguration({camera});
}

void VulkanRenderer2D::endScene() {
    spriteRenderingPass.end();
    postProcessingPass.draw();

    imGuiRenderingPass.draw();
    context.getCurrentPrimaryCommandBuffer().end();
}

void VulkanRenderer2D::recreateSwapChain() {
    std::cout << "Recreating SwapChain" << std::endl;
    context.getDevice().waitIdle();

    // Update framebuffer attachments
    spriteRenderingPass.resizeAttachments(context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
    imGuiRenderingPass.resizeAttachments(context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
    postProcessingPass.resizeAttachments(spriteRenderingPass.getFramebuffer());
}

void VulkanRenderer2D::flush() {
    if (!context.flushCommands()) {
        // Display surface has changed -> update framebuffer attachments
        recreateSwapChain();
    }

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void VulkanRenderer2D::draw(const glm::mat4 &modelMat, const RenderComponent &renderComponent) {
    spriteRenderingPass.drawSprite(quadMesh, modelMat,
                                   dynamic_cast<const Renderer::VulkanMaterialInstance &>(*(renderComponent.materialInstance)));
}

const Renderer::RenderPass &VulkanRenderer2D::getRenderPassForShaderStage(Renderer::ShaderPassStage stage) const {
    switch (stage) {
        case Renderer::ShaderPassStage::Opaque:
            return spriteRenderingPass.getOpaquePass();
        default:
            throw std::runtime_error("[Renderer] Shader stage is not supported in VulkanRenderer2D.");
    }
}
