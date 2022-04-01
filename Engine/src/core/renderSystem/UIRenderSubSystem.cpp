#include "UIRenderSubSystem.h"

#include "core/Components.h"
#include "core/utils/Logger.h"
#include "Engine/src/renderer/api/GraphicsContext.h"

using namespace ChaosEngine;
using namespace Renderer;

void UIRenderSubSystem::init(uint32_t pGlyphCapacity) {
    assert("Glyph capacity must be smaller than MAX_UINT32_T / 4 to accommodate the index buffer range!" &&
           pGlyphCapacity <= std::numeric_limits<uint32_t>::max() / 4);
    glyphCapacity = pGlyphCapacity;

    const size_t vertexBufferCapacity = 4 * sizeof(GlyphVertex) * glyphCapacity;
    const size_t indexBufferCapacity = 6 * sizeof(uint32_t) * glyphCapacity;
    std::vector<GlyphVertex> textVertexBufferCPU{4 * glyphCapacity, GlyphVertex{}};
    std::vector<uint32_t> textIndexBufferCPU{6 * glyphCapacity, 0};

    textVertexBuffers.reserve(GraphicsContext::maxFramesInFlight);
    textIndexBuffers.reserve(GraphicsContext::maxFramesInFlight);
    for (uint32_t i = 0; i < GraphicsContext::maxFramesInFlight; ++i) {
        textVertexBuffers.emplace_back(
                Buffer::CreateStreaming(textVertexBufferCPU.data(), vertexBufferCapacity, BufferType::Vertex));
        textIndexBuffers.emplace_back(
                Buffer::CreateStreaming(textIndexBufferCPU.data(), indexBufferCapacity, BufferType::Index));
    }

    LOG_INFO("UIRenderSubSystem: Creating materials");
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

uint32_t
UIRenderSubSystem::renderTextToBuffers(uint32_t bufferOffsetInGlyphs, GlyphVertex *vBufferRef, uint32_t *iBufferRef,
                                       const UITextComponent &text, glm::vec3 linePos) const {
    glm::vec3 cursor{0, -text.font->getLineHeight(), 0};
    uint32_t glyphCount = 0;
    for (char car: text.text) {
        if (car == '\n') {
            cursor = {0, cursor.y - text.font->getLineHeight(), 0};
            continue;
        }
        uint32_t totalGlyphCount = bufferOffsetInGlyphs + glyphCount;
        if (totalGlyphCount >= glyphCapacity) {
            LOG_ERROR("No more space in text buffers!");
            break;
        }
        const auto glyph = text.font->getGlyph(car);
        glm::vec3 glyphPos = linePos + cursor;
        glyphPos.x += glyph.bearing.x;
        glyphPos.y -= glyph.size.y - glyph.bearing.y;

        glm::vec2 uvBoxLT = glyph.uvOffset;
        glm::vec2 uvBoxRB = glyph.uvOffset + glyph.uvSize;
        vBufferRef[4 * totalGlyphCount + 0] = GlyphVertex{
                .pos = glyphPos,
                .color = text.textColor,
                .uv=glm::vec2(uvBoxLT.x, uvBoxRB.y)
        };
        vBufferRef[4 * totalGlyphCount + 1] = GlyphVertex{
                .pos = glyphPos + glm::vec3(0, glyph.size.y, 0),
                .color = text.textColor,
                .uv=uvBoxLT
        };
        vBufferRef[4 * totalGlyphCount + 2] = GlyphVertex{
                .pos = glyphPos + glm::vec3(glyph.size.x, 0, 0),
                .color = text.textColor,
                .uv=uvBoxRB
        };
        vBufferRef[4 * totalGlyphCount + 3] = GlyphVertex{
                .pos = glyphPos + glm::vec3(glyph.size.x, glyph.size.y, 0),
                .color = text.textColor,
                .uv=glm::vec2(uvBoxRB.x, uvBoxLT.y)
        };
        cursor.x += static_cast<float>(glyph.advance);
        iBufferRef[totalGlyphCount * 6 + 0] = (totalGlyphCount * 4) + 0;
        iBufferRef[totalGlyphCount * 6 + 1] = (totalGlyphCount * 4) + 2;
        iBufferRef[totalGlyphCount * 6 + 2] = (totalGlyphCount * 4) + 1;
        iBufferRef[totalGlyphCount * 6 + 3] = (totalGlyphCount * 4) + 2;
        iBufferRef[totalGlyphCount * 6 + 4] = (totalGlyphCount * 4) + 3;
        iBufferRef[totalGlyphCount * 6 + 5] = (totalGlyphCount * 4) + 1;
        ++glyphCount;
    }
    return glyphCount;
}

glm::mat4 calculateTextModelMatrix(const Transform &transform) {
    auto linePos = transform.position;
    linePos.y *= -1;
    glm::mat4 modelMat = glm::translate(glm::mat4(1), linePos);
    modelMat *= glm::toMat4(glm::quat({glm::radians(transform.rotation.x), glm::radians(transform.rotation.y),
                                       glm::radians(transform.rotation.z)}));
    return glm::scale(modelMat, transform.scale);;
}

void UIRenderSubSystem::render(ECS &ecs, Renderer::RendererAPI &renderer) {
    // Render Text
    auto uiTexts = ecs.getRegistry().view<Transform, UITextComponent>();
    uint32_t totalGlyphCount = 0;
    auto *vBufferRef = static_cast<GlyphVertex *>(textVertexBuffers[currentBufferedFrame]->map());
    auto *iBufferRef = static_cast<uint32_t *>(textIndexBuffers[currentBufferedFrame]->map());

    renderer.beginUI(glm::mat4(1.0f));
    for (auto&&[entity, transform, text]: uiTexts.each()) {
        if (fontMaterialInstance == nullptr) {
            // TODO: Multi font support
            auto fontTexture = text.font->getFontTexture();
            fontMaterialInstance = uiMaterial.instantiate(nullptr, 0, {fontTexture});
        }

        auto glyphCount = renderTextToBuffers(totalGlyphCount, vBufferRef, iBufferRef, text, glm::vec3());
        glm::mat4 modelMat = calculateTextModelMatrix(transform);
        renderer.drawUI(*textVertexBuffers[currentBufferedFrame], *textIndexBuffers[currentBufferedFrame],
                        glyphCount * 6, totalGlyphCount * 6, modelMat, *fontMaterialInstance);

        totalGlyphCount += glyphCount;
    }
    textVertexBuffers[currentBufferedFrame]->flush();
    textIndexBuffers[currentBufferedFrame]->flush();

    // Render UI elements
    // TODO

    renderer.endUI();

    currentBufferedFrame = (currentBufferedFrame + 1) % GraphicsContext::maxFramesInFlight;
}