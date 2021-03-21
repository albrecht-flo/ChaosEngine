#pragma once

#include <string>
#include <utility>
#include <memory>
#include <optional>
#include <vector>
#include <cassert>

#include "Engine/src/core/Components.h"
#include "Engine/src/renderer/api/RenderPass.h"
#include "Engine/src/renderer/api/Material.h"

namespace Renderer {

    enum class RendererType {
        RENDERER2D
    };

    class RendererAPI {
    public:
        virtual ~RendererAPI() = default;

        virtual void setup() = 0;

        virtual void beginScene(const glm::mat4 &viewMatrix, const CameraComponent &camera) = 0;

        virtual void draw(const glm::mat4 &viewMatrix, const RenderComponent &renderComponent) = 0;

        virtual void endScene() = 0;

        virtual void flush() = 0;

        virtual void join() = 0;

        virtual const RenderPass &getRenderPassForShaderStage(ShaderPassStage stage) const = 0;
    };
}