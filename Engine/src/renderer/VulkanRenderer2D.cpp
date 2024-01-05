#include "VulkanRenderer2D.h"

#include "core/utils/Logger.h"
#include "core/assets/Mesh.h"
#include "renderer/api/Material.h"
#include "renderer/api/GraphicsContext.h"
#include "renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "renderer/vulkan/rendering/VulkanAttachmentBuilder.h"

// ------------------------------------ Class Construction -------------------------------------------------------------

std::unique_ptr<VulkanRenderer2D>
VulkanRenderer2D::Create(Renderer::GraphicsContext& graphicsContext, bool renderingSceneToSwapchain,
                         bool enableDebugRendering) {
    auto& context = dynamic_cast<VulkanContext&>(graphicsContext);
    auto spriteRenderingPass = SpriteRenderingPass::Create(context, context.getSwapChain().getWidth(),
                                                           context.getSwapChain().getHeight());

    std::optional<DebugRenderingPass> debugRenderingPass =
        (enableDebugRendering)
            ? std::make_optional(DebugRenderingPass::Create(context, spriteRenderingPass.getOpaquePass(),
                                                            context.getSwapChain().getWidth(),
                                                            context.getSwapChain().getHeight()))
            : std::nullopt;

    auto uiRenderingPass = UIRenderingPass::Create(context, context.getSwapChain().getWidth(),
                                                   context.getSwapChain().getHeight());
    auto textRenderingPass = UIRenderingPass::Create(context, context.getSwapChain().getWidth(),
                                                     context.getSwapChain().getHeight());

    auto postProcessingPass = PostProcessingPass::Create(context, spriteRenderingPass.getFramebuffer(),
                                                         uiRenderingPass.getFramebuffer(),
                                                         textRenderingPass.getFramebuffer(),
                                                         renderingSceneToSwapchain,
                                                         context.getSwapChain().getWidth(),
                                                         context.getSwapChain().getHeight());

    auto imGuiRenderingPass = ImGuiRenderingPass::Create(context, context.getWindow(), !renderingSceneToSwapchain);

    return std::unique_ptr<VulkanRenderer2D>(
        new VulkanRenderer2D(context, std::move(spriteRenderingPass), std::move(debugRenderingPass),
                             std::move(uiRenderingPass), std::move(textRenderingPass),
                             std::move(postProcessingPass), std::move(imGuiRenderingPass),
                             renderingSceneToSwapchain, enableDebugRendering));
}

VulkanRenderer2D::VulkanRenderer2D(VulkanContext& context,
                                   SpriteRenderingPass&& spriteRenderingPass,
                                   std::optional<DebugRenderingPass>&& debugRenderingPass,
                                   UIRenderingPass&& uiRenderPass,
                                   UIRenderingPass&& textRenderPass,
                                   PostProcessingPass&& postProcessingPass,
                                   ImGuiRenderingPass&& imGuiRenderingPass,
                                   bool renderingSceneToSwapchain,
                                   bool debugRenderingEnabled)
    : context(context), spriteRenderingPass(std::move(spriteRenderingPass)),
      debugRenderingPass(std::move(debugRenderingPass)),
      uiRenderingPass(std::move(uiRenderPass)), textRenderingPass(std::move(textRenderPass)),
      postProcessingPass(std::move(postProcessingPass)), imGuiRenderingPass(std::move(imGuiRenderingPass)),
      renderingSceneToSwapchain(renderingSceneToSwapchain), debugRenderingEnabled(debugRenderingEnabled) {}


// ------------------------------------ Lifecycle methods --------------------------------------------------------------

void VulkanRenderer2D::setup() {
    if (debugRenderingEnabled) {
        for (uint32_t i = 0; i < Renderer::GraphicsContext::maxFramesInFlight; ++i) {
            std::vector<uint8_t> data;
            size_t size = sizeof(VertexPC) * maxDebugVertices;
            data.reserve(size);
            debugBuffers.emplace_back(std::unique_ptr<VulkanBuffer>(
                    dynamic_cast<VulkanBuffer*>(
                        VulkanBuffer::CreateStreaming(
                            data.data(),
                            size,
                            Renderer::BufferType::Vertex).release()))
            );
        }
    }
}


void VulkanRenderer2D::join() {
    context.getDevice().waitIdle();
}

// ------------------------------------ Rendering methods --------------------------------------------------------------

void VulkanRenderer2D::beginFrame() {
    auto& commandBuffer = context.getCurrentPrimaryCommandBuffer();
    commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
}

void VulkanRenderer2D::beginScene(const glm::mat4& viewMat, const CameraComponent& camera) {
    spriteRenderingPass.begin(viewMat, camera);
    postProcessingPass.updateConfiguration({camera});
}

void VulkanRenderer2D::endScene() {
    spriteRenderingPass.end();
}

void VulkanRenderer2D::endFrame() {
    postProcessingPass.draw();

    imGuiRenderingPass.draw();
    context.getCurrentPrimaryCommandBuffer().end();
}

void VulkanRenderer2D::beginUI(const glm::mat4& viewMat) {
    uiRenderingPass.begin(viewMat);
}

void VulkanRenderer2D::endUI() {
    uiRenderingPass.end();
}

void VulkanRenderer2D::beginTextOverlay(const glm::mat4& viewMat) {
    textRenderingPass.begin(viewMat);
}

void VulkanRenderer2D::endTextOverlay() {
    textRenderingPass.end();
}

void VulkanRenderer2D::recreateSwapChain() {
    Logger::D("VulkanRenderer2D", "Recreating SwapChain");
    context.getDevice().waitIdle();

    // Update framebuffer attachments
    if (renderingSceneToSwapchain) {
        spriteRenderingPass.resizeAttachments(context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
        if (debugRenderingPass.has_value())
            debugRenderingPass->resizeAttachments(context.getSwapChain().getWidth(),
                                                  context.getSwapChain().getHeight());
        uiRenderingPass.resizeAttachments(context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
        textRenderingPass.resizeAttachments(context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
        postProcessingPass.resizeAttachments(spriteRenderingPass.getFramebuffer(), uiRenderingPass.getFramebuffer(),
                                             textRenderingPass.getFramebuffer(),
                                             context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
    }
    else if (sceneResize != glm::uvec2{0, 0}) {
        Logger::D("VulkanRenderer2D", "Resizing scene viewport during swapchain recreation");
        spriteRenderingPass.resizeAttachments(sceneResize.x, sceneResize.y);
        if (debugRenderingPass.has_value())
            debugRenderingPass->resizeAttachments(sceneResize.x, sceneResize.y);
        uiRenderingPass.resizeAttachments(sceneResize.x, sceneResize.y);
        textRenderingPass.resizeAttachments(sceneResize.x, sceneResize.y);
        postProcessingPass.resizeAttachments(spriteRenderingPass.getFramebuffer(), uiRenderingPass.getFramebuffer(),
                                             textRenderingPass.getFramebuffer(),
                                             sceneResize.x, sceneResize.y);
        sceneResize = {0, 0};
    }
    imGuiRenderingPass.resizeAttachments(context.getSwapChain().getWidth(), context.getSwapChain().getHeight());
}

void VulkanRenderer2D::requestViewportResize(const glm::vec2& viewportSize) {
    assert("Can't resize scene viewport independently when rendering to swapchain!" && !renderingSceneToSwapchain);
    assert("Invalid viewport size, MUST be greater than 0!" && viewportSize.x > 0 && viewportSize.y > 0);
    sceneResize = {static_cast<uint32_t>(viewportSize.x), static_cast<uint32_t>(viewportSize.y)};
}

void VulkanRenderer2D::flush() {
    //    Logger::W("VulkanRenderer2D", "Flushing...");
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
        uiRenderingPass.resizeAttachments(sceneResize.x, sceneResize.y);
        textRenderingPass.resizeAttachments(sceneResize.x, sceneResize.y);
        postProcessingPass.resizeAttachments(spriteRenderingPass.getFramebuffer(), uiRenderingPass.getFramebuffer(),
                                             textRenderingPass.getFramebuffer(),
                                             sceneResize.x, sceneResize.y);
        sceneResize = {0, 0};
    }
}

void VulkanRenderer2D::draw(const glm::mat4& modelMat, const RenderComponent& renderComponent) {
    const auto& mesh = dynamic_cast<const VulkanRenderMesh&>(*(renderComponent.mesh));
    const auto& material = dynamic_cast<const VulkanMaterialInstance&>(*(renderComponent.materialInstance));
    spriteRenderingPass.drawSprite(mesh, modelMat, material);
}

void
VulkanRenderer2D::drawText(const Renderer::Buffer& vertexBuffer, const Renderer::Buffer& indexBuffer,
                           uint32_t indexCount, uint32_t indexOffset,
                           const glm::mat4& modelMat, const Renderer::MaterialInstance& materialInstance) {
    const auto& vulkanVBuffer = dynamic_cast<const VulkanBuffer&>(vertexBuffer);
    const auto& vulkanIBuffer = dynamic_cast<const VulkanBuffer&>(indexBuffer);
    const auto& vulkanMaterialI = dynamic_cast<const VulkanMaterialInstance&>(materialInstance);
    textRenderingPass.drawUI(vulkanVBuffer, vulkanIBuffer, indexCount, indexOffset, modelMat, vulkanMaterialI);
}

void VulkanRenderer2D::drawUI(const glm::mat4& modelMat, const Renderer::RenderMesh& mesh,
                              const Renderer::MaterialInstance& material) {
    const auto* vulkanVBuffer = dynamic_cast<const VulkanBuffer*>(mesh.getVertexBuffer());
    const auto* vulkanIBuffer = dynamic_cast<const VulkanBuffer*>(mesh.getIndexBuffer());
    const auto& vulkanMaterialI = dynamic_cast<const VulkanMaterialInstance&>(material);
    uiRenderingPass.drawUI(*vulkanVBuffer, *vulkanIBuffer, mesh.getIndexCount(), 0, modelMat, vulkanMaterialI);
}

void VulkanRenderer2D::drawSceneDebug(const glm::mat4& viewMat, const CameraComponent& camera,
                                      const Renderer::DebugRenderData& debugRenderData) {
    if (!debugRenderingPass.has_value())
        return;

    uint32_t currentFrame = context.getCurrentFrame();

    size_t size = debugRenderData.lines.size();
    if (size > maxDebugVertices) {
        LOG_WARN("Too many debug vertices {}/{} ignore overhead!", size, maxDebugVertices);
        size = maxDebugVertices;
    }

    debugBuffers[currentFrame]->copy((void*)debugRenderData.lines.data(),
                                     size * sizeof(VertexPC));

    debugRenderingPass->begin(viewMat, camera);

    debugRenderingPass->drawLines(*debugBuffers[currentFrame], static_cast<uint32_t>(debugRenderData.lines.size()));
}

const Renderer::RenderPass& VulkanRenderer2D::getRenderPassForShaderStage(Renderer::ShaderPassStage stage) const {
    switch (stage) {
    case Renderer::ShaderPassStage::Opaque:
        return spriteRenderingPass.getOpaquePass();
    default:
        throw std::runtime_error("[Renderer] Shader stage is not supported in VulkanRenderer2D.");
    }
}

