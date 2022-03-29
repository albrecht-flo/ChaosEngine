#include "UIRenderSubSystem.h"

#include "core/Components.h"
#include "core/utils/Logger.h"
#include "Engine/src/renderer/api/GraphicsContext.h"

using namespace ChaosEngine;
using namespace Renderer;

UIRenderSubSystem::~UIRenderSubSystem() {
    for (int i = 0; i < GraphicsContext::maxFramesInFlight; ++i) {
        textVertexBuffers[i]->unmap();
        textIndexBuffers[i]->unmap();
    }
}

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
}

void UIRenderSubSystem::render(ECS &ecs, Renderer::RendererAPI &renderer) {
    // Render Text
    auto scripts = ecs.getRegistry().view<Transform, UITextComponent>();
    uint32_t glyphCount = 0;
    uint32_t indexCount = 0;
    auto fontMaterialInstance = nullptr; // TODO
    auto *vBufferRef = static_cast<GlyphVertex *>(textVertexBuffers[currentBufferedFrame]->map());
    auto *iBufferRef = static_cast<uint32_t *>(textIndexBuffers[currentBufferedFrame]->map());
    for (auto&&[entity, transform, text]: scripts.each()) {
        glm::vec3 cursor{0, 0, 0};
        auto linePos = transform.position;
        for (char car: text.text) {
            if(car == '\n') {
                cursor = {0, cursor.y + text.font->getLineHeight(), 0};
                continue;
            }
            if (glyphCount >= glyphCapacity) {
                LOG_ERROR("No more space in text buffers!");
                break;
            }
            const auto glyph = text.font->getGlyph(car);
            auto fontTexture = text.font->getFontTexture();
            // TODO: Update material if necesary
            const auto fontSize = text.font->getSize();
            glm::vec3 glyphPos = linePos + cursor;
            vBufferRef[glyphCount + 0] = GlyphVertex{
                    .pos = glm::vec4(glyphPos, 1),
                    .color = text.textColor, .uv=glyph.uvOffset};
            vBufferRef[glyphCount + 1] = GlyphVertex{
                    .pos = glm::vec4(glyphPos + glm::vec3(0, fontSize * glyph.uvSize.y, 0), 1),
                    .color = text.textColor, .uv=glyph.uvOffset + glm::vec2(0, glyph.uvSize.y)};
            vBufferRef[glyphCount + 2] = GlyphVertex{
                    .pos = glm::vec4(glyphPos + glm::vec3(fontSize * glyph.uvSize.x, 0, 0), 1),
                    .color = text.textColor, .uv=glyph.uvOffset + glm::vec2(glyph.uvSize.x, 0)};
            vBufferRef[glyphCount + 3] = GlyphVertex{
                    .pos = glm::vec4(glyphPos +
                                     glm::vec3(fontSize * glyph.uvSize.x, fontSize * glyph.uvSize.y, 0), 1),
                    .color = text.textColor, .uv=glyph.uvOffset + glyph.uvSize};
            cursor.x += static_cast<float>(glyph.advance);
            iBufferRef[indexCount + 0] = indexCount + 0;
            iBufferRef[indexCount + 1] = indexCount + 1;
            iBufferRef[indexCount + 2] = indexCount + 2;
            iBufferRef[indexCount + 3] = indexCount + 2;
            iBufferRef[indexCount + 4] = indexCount + 1;
            iBufferRef[indexCount + 5] = indexCount + 3;
            indexCount += 6;
            ++glyphCount;
            break;
        }
    }
    textVertexBuffers[currentBufferedFrame]->flush();
    textIndexBuffers[currentBufferedFrame]->flush();

    // TODO implement render pass
    renderer.beginUI();
    if (glyphCount > 0) {
        renderer.drawUI(textVertexBuffer, textIndexBuffer, indexCount, fontMaterialInstance);
    }
    // Render UI elements
    // TODO

    renderer.endUI();
    currentBufferedFrame = (currentBufferedFrame + 1) % GraphicsContext::maxFramesInFlight;
}