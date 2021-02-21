#pragma once

#include <string>
#include <utility>
#include <memory>
#include <cassert>
#include "Engine/src/core/Components.h"

namespace Renderer {

    enum class RendererType {
        RENDERER2D
    };

    enum class ShaderPassStage {
        Opaque
    };

    enum class PolygonMode {
        Fill, Line, Point
    };
    enum class CullFace {
        CLW, CCLW
    };
    enum class CompareOp {
        Less, LessEqual, Greater, GreaterEqual, NotEqual, Equal, Always, Never
    };
    enum class Topology {
        PointList, LineList, LineStrip, TriangleList, TriangleStrip, TriangleFan
    };

    struct FixedFunctionConfiguration {
        Topology topology = Topology::TriangleList;
        PolygonMode polygonMode = PolygonMode::Fill;
        CullFace cullMode = CullFace::CCLW;
        bool depthTest;
        bool depthWrite;
    };

    struct MaterialCreateInfo {
        ShaderPassStage stage = ShaderPassStage::Opaque;
        // InputBindings
        FixedFunctionConfiguration fixedFunction;
        std::string vertexShader;
        std::string fragmentShader;
        uint32_t texturesBindingCount;
        uint32_t uniformBindingCount;
    };

    class RendererAPI {
    public:
        virtual void setup() = 0;

        virtual void beginScene(const glm::mat4 &viewMatrix, const CameraComponent &camera) = 0;

        virtual void draw(const glm::mat4 &viewMatrix, const RenderComponent &renderComponent) = 0;

        virtual void endScene() = 0;

        virtual void flush() = 0;

        virtual void join() = 0;
    };
}