#pragma once

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanPipelineBuilder.h"
#include "Engine/src/renderer/api/Material.h"

class VulkanDescriptorSetLayoutBuilder {
public:
    explicit VulkanDescriptorSetLayoutBuilder(const VulkanDevice &device)
            : device(device) {}

    ~VulkanDescriptorSetLayoutBuilder() = default;

    VulkanDescriptorSetLayoutBuilder &addBinding(uint32_t binding, Renderer::ShaderBindingType type,
                                                 Renderer::ShaderStage stage, uint32_t size = 1);

    VulkanDescriptorSetLayout build();

private:
    const VulkanDevice &device;
    std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;
};

namespace VulkanPipelineUtility {
    static inline VkShaderStageFlags getVkShaderStage(Renderer::ShaderStage stage) {
        switch (stage) {
            case Renderer::ShaderStage::Vertex:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case Renderer::ShaderStage::Fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case Renderer::ShaderStage::VertexFragment:
                return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            case Renderer::ShaderStage::Geometry:
                return VK_SHADER_STAGE_GEOMETRY_BIT;
            case Renderer::ShaderStage::TesselationControl:
                return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case Renderer::ShaderStage::TesselationEvaluation:
                return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case Renderer::ShaderStage::All:
                return VK_SHADER_STAGE_ALL_GRAPHICS;
        }
        assert("Unknown shader stage" && false);
        return VK_SHADER_STAGE_VERTEX_BIT;
    }

    static inline VkDescriptorType getVkDescriptorType(Renderer::ShaderBindingType type) {
        switch (type) {
            case Renderer::ShaderBindingType::UniformBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case Renderer::ShaderBindingType::TextureSampler:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        assert(false);
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}