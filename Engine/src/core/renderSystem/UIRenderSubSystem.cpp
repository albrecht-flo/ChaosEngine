#include "UIRenderSubSystem.h"

#include "core/Components.h"
#include "core/utils/Logger.h"
#include "Engine/src/renderer/api/GraphicsContext.h"

using namespace ChaosEngine;
using namespace Renderer;

void UIRenderSubSystem::init() {
    const size_t vertexBufferCapacity = 4 * sizeof(GlyphVertex) * glyphCapacity;
    const size_t indexBufferCapacity = 6 * sizeof(uint32_t) * glyphCapacity;
    std::vector<GlyphVertex> textVertexBufferCPU{4 * glyphCapacity, GlyphVertex{}};
    std::vector<uint32_t> textIndexBufferCPU{6 * glyphCapacity, 0};

    textVertexBuffers.reserve(GraphicsContext::maxFramesInFlight);
    textIndexBuffers.reserve(GraphicsContext::maxFramesInFlight);
    for (int i = 0; i < GraphicsContext::maxFramesInFlight; ++i) {
        textVertexBuffers.emplace_back(
                Buffer::CreateStreaming(textVertexBufferCPU.data(), vertexBufferCapacity, BufferType::Vertex));
        textIndexBuffers.emplace_back(
                Buffer::CreateStreaming(textIndexBufferCPU.data(), indexBufferCapacity, BufferType::Index));
    }

    LOG_INFO("Creating materials");
    uiMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .vertexLayout = VertexLayout{.binding = 0, .stride = sizeof(GlyphVertex), .inputRate=InputRate::Vertex,
                    .attributes = std::vector<VertexAttribute>(
                            {
                                    VertexAttribute{0, VertexFormat::RGB_FLOAT, offsetof(GlyphVertex, pos)},
                                    VertexAttribute{1, VertexFormat::RGBA_FLOAT, offsetof(GlyphVertex, color)},
                                    VertexAttribute{2, VertexFormat::RG_FLOAT, offsetof(GlyphVertex, uv)},
                            })},
            .fixedFunction = FixedFunctionConfiguration{.depthTest = true, .depthWrite = true},
            .vertexShader = "UI",
            .fragmentShader = "UI",
            .pushConstant = std::make_optional(Material::StandardOpaquePushConstants),
            .set0 = std::make_optional(Material::StandardOpaqueSet0),
            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
            .set1 = std::make_optional(std::vector<ShaderBindings>(
                    {
                            ShaderBindings{.type = ShaderBindingType::TextureSampler, .stage=ShaderStage::Fragment, .name="texture"},
                    })),
            .set1ExpectedCount = 64,
            .name="UIMaterial",
    });
}

void UIRenderSubSystem::render(ECS &ecs, Renderer::RendererAPI &renderer) {
    // Render Text
    auto scripts = ecs.getRegistry().view<UITextComponent>();
    uint32_t glyphCount = 0;
    auto *vBufferRef = static_cast<GlyphVertex *>(textVertexBuffers[currentBufferedFrame]->map());
    auto *iBufferRef = static_cast<uint32_t *>(textIndexBuffers[currentBufferedFrame]->map());
    for (auto&&[entity, text]: scripts.each()) {
        if (fontMaterialInstance == nullptr) {
            // TODO: Multi font support
            auto fontTexture = text.font->getFontTexture();
            fontMaterialInstance = uiMaterial.instantiate(nullptr, 0, {fontTexture});
        }


        glm::vec3 cursor{0, 0, 0};
        auto linePos = text.position;
        for (char car: text.text) {
            if (car == '\n') {
                cursor = {0, cursor.y - text.font->getLineHeight(), 0};
                continue;
            }
            if (glyphCount >= glyphCapacity) {
                LOG_ERROR("No more space in text buffers!");
                break;
            }
            const auto glyph = text.font->getGlyph(car);
            const auto fontSize = text.font->getSize();
            glm::vec3 glyphPos = linePos + cursor;
            vBufferRef[4 * glyphCount + 0] = GlyphVertex{
                    .pos = glyphPos,
                    .color = text.textColor, .uv=glyph.uvOffset};
            vBufferRef[4 * glyphCount + 1] = GlyphVertex{
                    .pos = glyphPos + glm::vec3(0, -glyph.size.y, 0),
                    .color = text.textColor, .uv=glyph.uvOffset + glm::vec2(0, glyph.uvSize.y)};
            vBufferRef[4 * glyphCount + 2] = GlyphVertex{
                    .pos = glyphPos + glm::vec3(glyph.size.x, 0, 0),
                    .color = text.textColor, .uv=glyph.uvOffset + glm::vec2(glyph.uvSize.x, 0)};
            vBufferRef[4 * glyphCount + 3] = GlyphVertex{
                    .pos = glyphPos + glm::vec3(glyph.size.x, -glyph.size.y, 0),
                    .color = text.textColor, .uv=glyph.uvOffset + glyph.uvSize};
            cursor.x += static_cast<float>(glyph.advance);
            iBufferRef[glyphCount * 6 + 0] = (glyphCount * 4) + 0;
            iBufferRef[glyphCount * 6 + 1] = (glyphCount * 4) + 1;
            iBufferRef[glyphCount * 6 + 2] = (glyphCount * 4) + 2;
            iBufferRef[glyphCount * 6 + 3] = (glyphCount * 4) + 2;
            iBufferRef[glyphCount * 6 + 4] = (glyphCount * 4) + 1;
            iBufferRef[glyphCount * 6 + 5] = (glyphCount * 4) + 3;
            ++glyphCount;
        }
    }
    textVertexBuffers[currentBufferedFrame]->flush();
    textIndexBuffers[currentBufferedFrame]->flush();

    renderer.beginUI(glm::mat4(1.0f));
    if (glyphCount > 0) {
        renderer.drawUI(*textVertexBuffers[currentBufferedFrame], *textIndexBuffers[currentBufferedFrame],
                        glyphCount * 6, glm::mat4(1.0f), *fontMaterialInstance);
    }
    // Render UI elements
    // TODO

    renderer.endUI();
    currentBufferedFrame = (currentBufferedFrame + 1) % GraphicsContext::maxFramesInFlight;
}