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

    const size_t vertexBufferCapacity = 4 * sizeof(VertexPCU) * glyphCapacity;
    const size_t indexBufferCapacity = 6 * sizeof(uint32_t) * glyphCapacity;
    std::vector<VertexPCU> textVertexBufferCPU{4 * glyphCapacity, VertexPCU{}};
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
    uiTextMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .vertexLayout = VertexLayout{.binding = 0, .stride = sizeof(VertexPCU), .inputRate=InputRate::Vertex,
                    .attributes = std::vector<VertexAttribute>(
                            {
                                    VertexAttribute{0, VertexFormat::RGB_FLOAT, offsetof(VertexPCU, pos)},
                                    VertexAttribute{1, VertexFormat::RGBA_FLOAT, offsetof(VertexPCU, color)},
                                    VertexAttribute{2, VertexFormat::RG_FLOAT, offsetof(VertexPCU, uv)},
                            })},
            .fixedFunction = FixedFunctionConfiguration{.depthTest = true, .depthWrite = true, .alphaBlending=true},
            .vertexShader = "UIBase",
            .fragmentShader = "UIText",
            .pushConstant = std::make_optional(Material::StandardOpaquePushConstants),
            .set0 = std::make_optional(Material::StandardOpaqueSet0),
            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
            .set1 = std::make_optional(std::vector<ShaderBindings>(
                    {
                            ShaderBindings{.type = ShaderBindingType::TextureSampler, .stage=ShaderStage::Fragment, .name="texture"},
                    })),
            .set1ExpectedCount = 64,
            .name="UITextMaterial",
    });
}

uint32_t
UIRenderSubSystem::renderTextToBuffers(uint32_t bufferOffsetInGlyphs, VertexPCU *vBufferRef, uint32_t *iBufferRef,
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
        vBufferRef[4 * totalGlyphCount + 0] = VertexPCU{
                .pos = glyphPos,
                .color = text.textColor,
                .uv=glm::vec2(uvBoxLT.x, uvBoxRB.y)
        };
        vBufferRef[4 * totalGlyphCount + 1] = VertexPCU{
                .pos = glyphPos + glm::vec3(0, glyph.size.y, 0),
                .color = text.textColor,
                .uv=uvBoxLT
        };
        vBufferRef[4 * totalGlyphCount + 2] = VertexPCU{
                .pos = glyphPos + glm::vec3(glyph.size.x, 0, 0),
                .color = text.textColor,
                .uv=uvBoxRB
        };
        vBufferRef[4 * totalGlyphCount + 3] = VertexPCU{
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

static glm::mat4
calculateTextModelMatrix(const Transform &transform,
                         const glm::vec3 scaleOffset = glm::vec3(0),
                         const UIComponent &uiC = UIComponent{false, glm::vec3(0), glm::vec3(0), glm::vec3(0)}) {
    auto pos = transform.position + uiC.offsetPosition;
    const auto rot = transform.rotation + uiC.offsetRotation;
    const auto scl = transform.scale + uiC.offsetScale + scaleOffset;
    pos.y *= -1;
    glm::mat4 modelMat = glm::translate(glm::mat4(1), pos);
    modelMat *= glm::toMat4(glm::quat(glm::vec3{glm::radians(rot.x), glm::radians(rot.y),
                                                glm::radians(rot.z)}));
    return glm::scale(modelMat, scl);
}

void UIRenderSubSystem::render(ECS &ecs, Renderer::RendererAPI &renderer) {
    // Render Text
    auto uiTexts = ecs.getRegistry().view<Transform, UITextComponent>();
    uint32_t totalGlyphCount = 0;
    auto *vBufferRef = static_cast<VertexPCU *>(textVertexBuffers[currentBufferedFrame]->map());
    auto *iBufferRef = static_cast<uint32_t *>(textIndexBuffers[currentBufferedFrame]->map());

    renderer.beginUI(glm::mat4(1.0f));
    for (auto&&[entity, transform, text]: uiTexts.each()) {
        if (!fontMaterialInstances.contains(text.font.get())) {
            auto fontTexture = text.font->getFontTexture();
            fontMaterialInstances[text.font.get()] = uiTextMaterial.instantiate(nullptr, 0, {fontTexture});
        }

        auto glyphCount = renderTextToBuffers(totalGlyphCount, vBufferRef, iBufferRef, text, glm::vec3());
        glm::mat4 modelMat = calculateTextModelMatrix(transform);
        renderer.drawUI(*textVertexBuffers[currentBufferedFrame], *textIndexBuffers[currentBufferedFrame],
                        glyphCount * 6, totalGlyphCount * 6, modelMat, *fontMaterialInstances[text.font.get()]);

        totalGlyphCount += glyphCount;
    }
    textVertexBuffers[currentBufferedFrame]->flush();
    textIndexBuffers[currentBufferedFrame]->flush();

    // Render UI elements
    const auto uiComps = ecs.getRegistry().view<const Transform, const UIRenderComponent>();
    for (auto&&[entity, transform, ui]: uiComps.each()) {
        const auto *uiC = ecs.getRegistry().try_get<UIComponent>(entity);
        if (uiC == nullptr)
            renderer.drawUI(calculateTextModelMatrix(transform, ui.scaleOffset), *ui.mesh, *ui.materialInstance);
        else
            renderer.drawUI(calculateTextModelMatrix(transform, ui.scaleOffset, *uiC), *ui.mesh, *ui.materialInstance);
    }

    renderer.endUI();

    currentBufferedFrame = (currentBufferedFrame + 1) % GraphicsContext::maxFramesInFlight;
}