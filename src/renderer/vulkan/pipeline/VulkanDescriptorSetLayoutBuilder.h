#pragma once

#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanDescriptorSetLayout.h"

enum class ShaderStage {
    Vertex, Fragment, VertexFragment, Geometry, TesselationControl, TesselationEvaluation, All
};
enum class DescriptorType {
    UniformBuffer, Texture
};

class VulkanDescriptorSetLayoutBuilder {
public:
    explicit VulkanDescriptorSetLayoutBuilder(const VulkanDevice &device)
            : device(device) {}

    ~VulkanDescriptorSetLayoutBuilder() = default;

    VulkanDescriptorSetLayoutBuilder &addBinding(uint32_t binding, DescriptorType type,
                                                 ShaderStage stage, uint32_t size = 1);

    VulkanDescriptorSetLayout build();

private:
    const VulkanDevice &device;
    std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;
};

namespace VulkanPipelineUtility {
    static inline VkShaderStageFlags getVkShaderStage(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::Fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderStage::VertexFragment:
                return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderStage::Geometry:
                return VK_SHADER_STAGE_GEOMETRY_BIT;
            case ShaderStage::TesselationControl:
                return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case ShaderStage::TesselationEvaluation:
                return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case ShaderStage::All:
                return VK_SHADER_STAGE_ALL_GRAPHICS;
        }
        assert("Unknown shader stage");
        return VK_SHADER_STAGE_VERTEX_BIT;
    }

    static inline VkDescriptorType getVkDescriptorType(DescriptorType type) {
        switch (type) {
            case DescriptorType::UniformBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case DescriptorType::Texture:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        assert(false);
    }
}