#pragma once

#include <glm/glm.hpp>

#include <string>
#include <utility>
#include <memory>
#include <optional>
#include <vector>
#include <cassert>
#include "Texture.h"

namespace Renderer {

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
    enum class ShaderStage {
        Vertex, Fragment, VertexFragment, Geometry, TesselationControl, TesselationEvaluation, All
    };
    enum class ShaderBindingType {
        UniformBuffer, TextureSampler
    };
    enum class ShaderValueType {
        Vec4, Mat4
    };

    inline uint32_t getSizeOfShaderValueType(ShaderValueType type) {
        switch (type) {
            case ShaderValueType::Vec4:
                return sizeof(glm::vec4);
            case ShaderValueType::Mat4:
                return sizeof(glm::mat4);
        }
    }

    struct ShaderBindingLayout {
        ShaderValueType type;
        std::string name;
    };
    struct ShaderPushConstantLayout {
        ShaderValueType type;
        ShaderStage stage;
        uint32_t offset;
        std::string name;
    };
    struct ShaderBindings {
        ShaderBindingType type;
        ShaderStage stage;
        std::string name;
        std::optional<std::vector<ShaderBindingLayout>> layout;
    };
    struct MaterialCreateInfo {
        ShaderPassStage stage = ShaderPassStage::Opaque;
        // InputBindings
        FixedFunctionConfiguration fixedFunction;
        std::string vertexShader;
        std::string fragmentShader;
        std::optional<std::vector<ShaderPushConstantLayout>> pushConstant;
        std::optional<std::vector<ShaderBindings>> set0;
        uint32_t set0ExpectedCount;
        std::optional<std::vector<ShaderBindings>> set1;
        uint32_t set1ExpectedCount;
    };


    class MaterialInstance {
    public:
        virtual ~MaterialInstance() = default;
    };

    class Material {
    public:
        virtual ~Material() = default;

        static std::unique_ptr<Material> Create(const MaterialCreateInfo &info);

        virtual std::unique_ptr<MaterialInstance>
        instantiate(const void *materialData, uint32_t size, const std::vector<const Texture *> &textures) = 0;

    public:
        static std::vector<ShaderBindings> StandardOpaqueSet0;
        static std::vector<ShaderPushConstantLayout> StandardOpaquePushConstants;
        static constexpr uint32_t StandardOpaqueSet0ExpectedCount = 2; // VulkanContext::maxFramesInFlight;
    };
}