#include "VulkanRenderer2D.h"

#include "Engine/src/core/Utils/Logger.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "Engine/src/core/assets/Mesh.h"
#include "Engine/src/core/assets/ModelLoader.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanAttachmentBuilder.h"
#include "Engine/src/renderer/api/Material.h"

#include <imgui.h>

// ------------------------------------ Class Construction -------------------------------------------------------------

std::unique_ptr<VulkanRenderer2D> VulkanRenderer2D::Create(Renderer::GraphicsContext &graphicsContext) {
    bool renderingSceneToSwapchain = false;
    auto &context = dynamic_cast<VulkanContext &>(graphicsContext);
    auto spriteRenderingPass = SpriteRenderingPass::Create(context, context.getSwapChain().getWidth(),
                                                           context.getSwapChain().getHeight());


    auto postProcessingPass = PostProcessingPass::Create(context, spriteRenderingPass.getFramebuffer(),
                                                         renderingSceneToSwapchain,
                                                         context.getSwapChain().getWidth(),
                                                         context.getSwapChain().getHeight());

    IMGUI_CHECKVERSION();
    ImGuiContext *imGuiContext = ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable new Viewport feature
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable new Docking feature
    ImGui::StyleColorsDark();
    auto imGuiRenderingPass = ImGuiRenderingPass::Create(context, context.getWindow(), imGuiContext);

    return std::unique_ptr<VulkanRenderer2D>(
            new VulkanRenderer2D(context, std::move(spriteRenderingPass), std::move(postProcessingPass),
                                 std::move(imGuiRenderingPass), renderingSceneToSwapchain));
}

VulkanRenderer2D::VulkanRenderer2D(VulkanContext &context, SpriteRenderingPass &&spriteRenderingPass,
                                   PostProcessingPass &&postProcessingPass, ImGuiRenderingPass &&imGuiRenderingPass,
                                   bool renderingSceneToSwapchain)
        : context(context), spriteRenderingPass(std::move(spriteRenderingPass)),
          postProcessingPass(std::move(postProcessingPass)), imGuiRenderingPass(std::move(imGuiRenderingPass)),
          renderingSceneToSwapchain(renderingSceneToSwapchain) {}


// ------------------------------------ Lifecycle methods --------------------------------------------------------------

void VulkanRenderer2D::setup() {
//    auto quad = ModelLoader::getQuad();
//    VulkanBuffer vertexBuffer = context.getMemory().createInputBuffer(
//            quad.vertices.size() * sizeof(quad.vertices[0]), reinterpret_cast<const char *>(quad.vertices.data()),
//            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
//
//    VulkanBuffer indexBuffer = context.getMemory().createInputBuffer(
//            quad.indices.size() * sizeof(quad.indices[0]), reinterpret_cast<const char *>(quad.indices.data()),
//            VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
//
//    quadMesh = std::make_unique<RenderMesh>(
//            std::move(vertexBuffer), std::move(indexBuffer), static_cast<uint32_t>(quad.indices.size()));
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
    Logger::D("VulkanRenderer2D", "Recreating SwapChain");
    context.getDevice().waitIdle();

    // Update framebuffer attachments
    if (renderingSceneToSwapchain) {
        spriteRenderingPass.resizeAttachments(context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
        postProcessingPass.resizeAttachments(spriteRenderingPass.getFramebuffer(),
                                             context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
    } else if (sceneResize != glm::uvec2{0, 0}) {
        Logger::D("VulkanRenderer2D", "Resizing scene viewport during swapchain recreation");
        spriteRenderingPass.resizeAttachments(sceneResize.x, sceneResize.y);
        postProcessingPass.resizeAttachments(spriteRenderingPass.getFramebuffer(), sceneResize.x, sceneResize.y);
        sceneResize = {0, 0};

    }
    imGuiRenderingPass.resizeAttachments(context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
}

void VulkanRenderer2D::requestViewportResize(const glm::vec2 &viewportSize) {
    assert("Can't resize scene viewport independently when rendering to swapchain!" && !renderingSceneToSwapchain);
    assert("Invalid viewport size, MUST be greater than 0!" && viewportSize.x > 0 && viewportSize.y > 0);
    sceneResize = {static_cast<uint32_t>(viewportSize.x), static_cast<uint32_t>(viewportSize.y)};
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

    if (sceneResize != glm::uvec2{0, 0}) {
        Logger::D("VulkanRenderer2D", "Resizing scene viewport");
        context.getDevice().waitIdle();
        spriteRenderingPass.resizeAttachments(sceneResize.x, sceneResize.y);
        postProcessingPass.resizeAttachments(spriteRenderingPass.getFramebuffer(), sceneResize.x, sceneResize.y);
        sceneResize = {0, 0};
    }
}

void VulkanRenderer2D::draw(const glm::mat4 &modelMat, const RenderComponent &renderComponent) {
    const auto &mesh = dynamic_cast<const VulkanRenderMesh &>(*(renderComponent.mesh));
    const auto &material = dynamic_cast<const VulkanMaterialInstance &>(*(renderComponent.materialInstance));
    spriteRenderingPass.drawSprite(mesh, modelMat, material);
}

const Renderer::RenderPass &VulkanRenderer2D::getRenderPassForShaderStage(Renderer::ShaderPassStage stage) const {
    switch (stage) {
        case Renderer::ShaderPassStage::Opaque:
            return spriteRenderingPass.getOpaquePass();
        default:
            throw std::runtime_error("[Renderer] Shader stage is not supported in VulkanRenderer2D.");
    }
}
