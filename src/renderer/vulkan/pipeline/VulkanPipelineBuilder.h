#pragma once

#include "VulkanVertexInput.h"
#include "VulkanPipeline.h"

#include <string>
#include <utility>
#include <cassert>

class VulkanRenderPass;

enum class Topology {
    PointList, LineList, LineStrip, TriangleList, TriangleStrip, TriangleFan
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

class VulkanPipelineBuilder {
public:
    VulkanPipelineBuilder(const VulkanDevice &device, const VulkanRenderPass &renderPass, const std::string &shaderName,
                          VulkanVertexInput input)
            : device(device), renderPass(renderPass), vertexShaderName(shaderName), fragmentShaderName(shaderName),
              vertexInput(std::move(input)) {}

    ~VulkanPipelineBuilder() = default;

    VulkanPipeline build();

    VulkanPipelineBuilder &setVertexShader(const std::string &pVertexShaderName) {
        vertexShaderName = pVertexShaderName;
        return *this;
    }

    VulkanPipelineBuilder &setFragmentShader(const std::string &pFragmentShaderName) {
        fragmentShaderName = pFragmentShaderName;
        return *this;
    }

    VulkanPipelineBuilder &setVertexInput(const VulkanVertexInput &pVertexInput) {
        vertexInput = pVertexInput;
        return *this;
    }

    VulkanPipelineBuilder &setTopology(Topology pTopology, bool pPrimitiveRestart = false) {
        topology = pTopology;
        primitiveRestart = pPrimitiveRestart;
        return *this;
    }

    VulkanPipelineBuilder &setPolygonMode(PolygonMode pPolygonMode) {
        polygonMode = pPolygonMode;
        return *this;
    }

    VulkanPipelineBuilder &setCullFace(CullFace pCullFace) {
        cullFace = pCullFace;
        return *this;
    }

    VulkanPipelineBuilder &setDepthTestEnabled(bool mDepthTestEnabled) {
        depthTestEnabled = mDepthTestEnabled;
        return *this;
    }

    VulkanPipelineBuilder &setDepthCompare(CompareOp pDepthCompare) {
        depthCompare = pDepthCompare;
        return *this;
    }

private:
    const VulkanDevice &device;
    const VulkanRenderPass &renderPass;
    std::string vertexShaderName;
    std::string fragmentShaderName;
    VulkanVertexInput vertexInput;
    Topology topology = Topology::TriangleList;
    bool primitiveRestart = false;
    PolygonMode polygonMode = PolygonMode::Fill;
    CullFace cullFace = CullFace::CCLW;
    bool depthTestEnabled = true;
    CompareOp depthCompare = CompareOp::Less;

private: // Translation helpers
    static VkPrimitiveTopology getVkTopology(Topology topology) {
        switch (topology) {
            case Topology::PointList:
                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case Topology::LineList:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case Topology::LineStrip:
                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case Topology::TriangleList:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case Topology::TriangleStrip:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case Topology::TriangleFan:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        }
        assert("Unknown Primitive type");
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }

    static VkPolygonMode getVkPolygonMode(PolygonMode polygonMode) {
        switch (polygonMode) {
            case PolygonMode::Fill:
                return VK_POLYGON_MODE_FILL;
            case PolygonMode::Line:
                return VK_POLYGON_MODE_LINE;
            case PolygonMode::Point:
                return VK_POLYGON_MODE_POINT;
        }
        assert("Unknown Polygon Mode");
        return VK_POLYGON_MODE_POINT;
    }

    static VkFrontFace getVkCullFace(CullFace cullFace) {
        switch (cullFace) {
            case CullFace::CLW:
                return VK_FRONT_FACE_COUNTER_CLOCKWISE;
            case CullFace::CCLW:
                return VK_FRONT_FACE_CLOCKWISE;
        }
        assert("Unknown Cull Type");
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    static VkCompareOp getVkCompareOp(CompareOp compareOp) {
        switch (compareOp) {
            case CompareOp::Less:
                return VK_COMPARE_OP_LESS;
            case CompareOp::LessEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case CompareOp::Greater:
                return VK_COMPARE_OP_GREATER;
            case CompareOp::GreaterEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case CompareOp::Equal:
                return VK_COMPARE_OP_EQUAL;
            case CompareOp::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;
            case CompareOp::Always:
                return VK_COMPARE_OP_ALWAYS;
            case CompareOp::Never:
                return VK_COMPARE_OP_NEVER;
        }
        assert("Unknown Compare Operation");
        return VK_COMPARE_OP_ALWAYS;
    }
};

