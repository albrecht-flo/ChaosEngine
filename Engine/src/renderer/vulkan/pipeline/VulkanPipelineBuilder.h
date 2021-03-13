#pragma once

#include "VulkanVertexInput.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanPipelineLayout.h"
#include "Engine/src/renderer/api/RendererAPI.h"
#include "Engine/src/renderer/api/Material.h"

#include <string>
#include <utility>
#include <memory>
#include <cassert>

class VulkanRenderPass;

class VulkanPipelineBuilder {
public:
    VulkanPipelineBuilder(const VulkanDevice &device, const VulkanRenderPass &renderPass,
                          VulkanPipelineLayout &&layout, VulkanVertexInput input,
                          const std::string &shaderName)
            : device(device), renderPass(renderPass), layout(std::move(layout)), vertexShaderName(shaderName),
              fragmentShaderName(shaderName), vertexInput(std::move(input)) {}

    ~VulkanPipelineBuilder() = default;

    VulkanPipeline build();

    VulkanPipelineBuilder &setLayout(VulkanPipelineLayout &&pLayout) {
        layout = std::move(pLayout);
        layoutValid = true;
        return *this;
    }

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

    VulkanPipelineBuilder &setTopology(Renderer::Topology pTopology, bool pPrimitiveRestart = false) {
        topology = pTopology;
        primitiveRestart = pPrimitiveRestart;
        return *this;
    }

    VulkanPipelineBuilder &setPolygonMode(Renderer::PolygonMode pPolygonMode) {
        polygonMode = pPolygonMode;
        return *this;
    }

    VulkanPipelineBuilder &setCullFace(Renderer::CullFace pCullFace) {
        cullFace = pCullFace;
        return *this;
    }

    VulkanPipelineBuilder &setDepthTestEnabled(bool mDepthTestEnabled) {
        depthTestEnabled = mDepthTestEnabled;
        return *this;
    }

    VulkanPipelineBuilder &setDepthCompare(Renderer::CompareOp pDepthCompare) {
        depthCompare = pDepthCompare;
        return *this;
    }

    VulkanPipelineBuilder &setViewportDimensions(uint32_t width, uint32_t height) {
        viewportExtent.width = width;
        viewportExtent.height = height;
        return *this;
    }

private:
    const VulkanDevice &device;
    const VulkanRenderPass &renderPass;
    VulkanPipelineLayout layout;
    std::string vertexShaderName;
    std::string fragmentShaderName;
    VulkanVertexInput vertexInput;
    Renderer::Topology topology = Renderer::Topology::TriangleList;
    bool primitiveRestart = false;
    Renderer::PolygonMode polygonMode = Renderer::PolygonMode::Fill;
    Renderer::CullFace cullFace = Renderer::CullFace::CCLW;
    bool depthTestEnabled = true;
    Renderer::CompareOp depthCompare = Renderer::CompareOp::Less;
    VkExtent2D viewportExtent{0, 0};
    // Internal error catching ----------------------------
    bool layoutValid = true;

private: // Translation helpers
    static VkPrimitiveTopology getVkTopology(Renderer::Topology topology) {
        switch (topology) {
            case Renderer::Topology::PointList:
                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case Renderer::Topology::LineList:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case Renderer::Topology::LineStrip:
                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case Renderer::Topology::TriangleList:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case Renderer::Topology::TriangleStrip:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case Renderer::Topology::TriangleFan:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        }
        assert("Unknown Primitive type");
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }

    static VkPolygonMode getVkPolygonMode(Renderer::PolygonMode polygonMode) {
        switch (polygonMode) {
            case Renderer::PolygonMode::Fill:
                return VK_POLYGON_MODE_FILL;
            case Renderer::PolygonMode::Line:
                return VK_POLYGON_MODE_LINE;
            case Renderer::PolygonMode::Point:
                return VK_POLYGON_MODE_POINT;
        }
        assert("Unknown Polygon Mode");
        return VK_POLYGON_MODE_POINT;
    }

    static VkFrontFace getVkCullFace(Renderer::CullFace cullFace) {
        switch (cullFace) {
            case Renderer::CullFace::CCLW:
                return VK_FRONT_FACE_COUNTER_CLOCKWISE;
            case Renderer::CullFace::CLW:
                return VK_FRONT_FACE_CLOCKWISE;
        }
        assert("Unknown Cull Type");
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    static VkCompareOp getVkCompareOp(Renderer::CompareOp compareOp) {
        switch (compareOp) {
            case Renderer::CompareOp::Less:
                return VK_COMPARE_OP_LESS;
            case Renderer::CompareOp::LessEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case Renderer::CompareOp::Greater:
                return VK_COMPARE_OP_GREATER;
            case Renderer::CompareOp::GreaterEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case Renderer::CompareOp::Equal:
                return VK_COMPARE_OP_EQUAL;
            case Renderer::CompareOp::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;
            case Renderer::CompareOp::Always:
                return VK_COMPARE_OP_ALWAYS;
            case Renderer::CompareOp::Never:
                return VK_COMPARE_OP_NEVER;
        }
        assert("Unknown Compare Operation");
        return VK_COMPARE_OP_ALWAYS;
    }
};

