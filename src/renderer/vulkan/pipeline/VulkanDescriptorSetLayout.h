#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <cassert>

class VulkanDevice;

class VulkanDescriptorSetLayout;

enum class ShaderStage {
    Vertex, Fragment, VertexFragment, Geometry, TesselationControl, TesselationEvaluation, All
};

enum class DescriptorType {
    UniformBuffer, Texture
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


class VulkanDescriptorSetLayout {
    friend VulkanDescriptorSetLayoutBuilder;
private:
    VulkanDescriptorSetLayout(const VulkanDevice &device, VkDescriptorSetLayout layout,
                              std::vector<VkDescriptorSetLayoutBinding> descriptorBindings)
            : device(device), layout(layout), descriptorBindings(std::move(descriptorBindings)) {}

    void destroy();

public:
    ~VulkanDescriptorSetLayout() { destroy(); }

    VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &o) = delete;

    VulkanDescriptorSetLayout &operator=(const VulkanDescriptorSetLayout &o) = delete;

    VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&o) noexcept
            : device(o.device), layout(std::exchange(o.layout, nullptr)),
              descriptorBindings(std::move(o.descriptorBindings)) {}

    VulkanDescriptorSetLayout &operator=(VulkanDescriptorSetLayout &&o) = delete;

    [[nodiscard]] inline VkDescriptorSetLayout vk() const { return layout; }

    inline VkDescriptorSetLayoutBinding getBinding(uint32_t binding) {
        assert(binding < descriptorBindings.size());
        return descriptorBindings[binding];
    }

private:
    const VulkanDevice &device;
    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;
};

// ------------------------------------ Pipeline Layout ----------------------------------------------------------------
class VulkanPipelineLayout;

class VulkanPipelineLayoutBuilder {
public:
    explicit VulkanPipelineLayoutBuilder(const VulkanDevice &device)
            : device(device) {}

    ~VulkanPipelineLayoutBuilder() = default;

    VulkanPipelineLayoutBuilder &addPushConstant(uint32_t size, uint32_t offset, ShaderStage stage);

    VulkanPipelineLayout build();


    VulkanPipelineLayoutBuilder &addDescriptorSet(const VulkanDescriptorSetLayout &layout);

private:
    const VulkanDevice &device;
    std::vector<VkDescriptorSetLayout> layouts;
    std::vector<VkPushConstantRange> pushConstants;
};


class VulkanPipelineLayout {
    friend VulkanPipelineLayoutBuilder;
private:
    VulkanPipelineLayout(const VulkanDevice &device, VkPipelineLayout layout)
            : device(device), layout(layout) {}

    void destroy();

public:
    ~VulkanPipelineLayout() { destroy(); }

    VulkanPipelineLayout(const VulkanPipelineLayout &o) = delete;

    VulkanPipelineLayout &operator=(const VulkanPipelineLayout &o) = delete;

    VulkanPipelineLayout(VulkanPipelineLayout &&o) noexcept
            : device(o.device), layout(std::exchange(o.layout, nullptr)) {}

    VulkanPipelineLayout &operator=(VulkanPipelineLayout &&o) noexcept;

    [[nodiscard]] inline VkPipelineLayout vk() const { return layout; }

private:
    const VulkanDevice &device;
    VkPipelineLayout layout;
};