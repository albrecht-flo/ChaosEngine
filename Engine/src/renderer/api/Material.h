#pragma once

#include "Texture.h"

#include <glm/glm.hpp>

#include <string>
#include <utility>
#include <memory>
#include <optional>
#include <vector>
#include <cassert>

namespace Renderer {
// ----------------------------- Vertex Input Configuration ------------------------------------------------------------
    enum class VertexFormat {
        R_FLOAT, RG_FLOAT, RGB_FLOAT, RGBA_FLOAT
    };
    enum class InputRate {
        Vertex, Instance
    };
    struct VertexAttribute {
        uint32_t location;
        VertexFormat format;
        size_t offset;
    };
    struct VertexLayout {
        uint32_t binding;
        uint32_t stride;
        InputRate inputRate;
        std::vector<VertexAttribute> attributes;
    };

// ----------------------------- Fixed Function Configuration ----------------------------------------------------------
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
        bool alphaBlending = false;
    };

// --------------------------------- Shader Configuration --------------------------------------------------------------
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
            default:
                assert("Unknown Shader Value Type!" && false);
                return 0;
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
        std::optional<std::vector<ShaderBindingLayout>> layout = std::nullopt;
    };

// --------------------------------- Material Configuration ------------------------------------------------------------
    struct MaterialCreateInfo {
        ShaderPassStage stage = ShaderPassStage::Opaque;
        // InputBindings
        VertexLayout vertexLayout;
        FixedFunctionConfiguration fixedFunction;
        std::string vertexShader;
        std::string fragmentShader;
        std::optional<std::vector<ShaderPushConstantLayout>> pushConstant;
        std::optional<std::vector<ShaderBindings>> set0;
        uint32_t set0ExpectedCount;
        std::optional<std::vector<ShaderBindings>> set1;
        uint32_t set1ExpectedCount;
        std::string name;
    };

// ------------------------------------ Material classes ---------------------------------------------------------------
    class GraphicsContext;

    /**
     * A MaterialInstance is a collection of Textures and material parameters that can be assigned to an Entity or
     * multiple Entities and is used by the Renderer to configure the shaders.
     */
    class MaterialInstance {
    public:
        virtual ~MaterialInstance() = default;
    };

    class MaterialRef;

    /**
     * A Material is a container for material instances. <br>
     * It therefore acts as a blueprint from which material instances can be created.
     * Therefore it also manages the uniform buffer storage on the GPU that is required for this material.
     */
    class Material {
    protected:
        explicit Material(GraphicsContext &context) : context(context) {}

    public:
        virtual ~Material() = default;

        static MaterialRef Create(const MaterialCreateInfo &info);

        /**
         * This method creates an instance of this material which can be assigned to an entity.
         *
         * @param materialPtr
         * @param materialData
         * @param size
         * @param textures
         * @return
         */
        virtual std::shared_ptr<MaterialInstance>
        instantiate(std::shared_ptr<Material> &materialPtr, const void *materialData, uint32_t size,
                    const std::vector<const Texture *> &textures) = 0;

        GraphicsContext &getContext() { return context; }

        virtual const std::string &getName() const = 0;

    protected:
        GraphicsContext &context;

    public:
        static std::vector<ShaderBindings> StandardOpaqueSet0;
        static std::vector<ShaderPushConstantLayout> StandardOpaquePushConstants;
        static constexpr uint32_t StandardOpaqueSet0ExpectedCount = 2; // VulkanContext::maxFramesInFlight;

    };

    /**
     * This class is a convenience wrapper around a shared_ptr to a Material to simplify the interface.
     */
    class MaterialRef {
    public:
        explicit MaterialRef(std::shared_ptr<Material> ptr) : pointer(std::move(ptr)) {}

        ~MaterialRef() = default;

        /**
         * This method instantiates the contained material with the supplied material and texture data.
         */
        inline std::shared_ptr<MaterialInstance>
        instantiate(const void *materialData, uint32_t size, const std::vector<const Texture *> &textures) {
            return pointer->instantiate(pointer, materialData, size, textures);
        }

        auto operator->() { return pointer.operator->(); }

    private:
        std::shared_ptr<Material> pointer;
    };

}