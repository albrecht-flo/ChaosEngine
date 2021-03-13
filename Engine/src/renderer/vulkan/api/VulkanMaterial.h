#pragma once

#include <Engine/src/renderer/data/Mesh.h>
#include <Engine/src/renderer/vulkan/context/VulkanContext.h>
#include <Engine/src/renderer/vulkan/pipeline/VulkanVertexInput.h>
#include <Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSetLayout.h>
#include <Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPool.h>
#include <Engine/src/renderer/vulkan/pipeline/VulkanPipeline.h>
#include <Engine/src/renderer/vulkan/image/VulkanTexture.h>
#include "Engine/src/renderer/api/Material.h"
#include "Engine/src/renderer/api/RendererAPI.h"

namespace Renderer {
    class VulkanMaterialInstance;

    // TODO: Double buffering of Descriptor sets
    class VulkanMaterial : public Material {
        friend class VulkanMaterialInstance;

    public:
        VulkanMaterial(const GraphicsContext &pContext, const RendererAPI &renderer, MaterialCreateInfo info);

        std::unique_ptr<MaterialInstance>
        instantiate(const void *materialData, uint32_t size, const std::vector<const Texture *> &textures) override;

    private:
        const VulkanContext &context;
        MaterialCreateInfo info;
        std::optional<std::unique_ptr<VulkanDescriptorSetLayout>> set0 = std::nullopt;
        std::optional<std::unique_ptr<VulkanDescriptorSetLayout>> set1 = std::nullopt;
        uint32_t materialBufferSize = 0;
        std::unique_ptr<VulkanPipeline> pipeline;
        std::unique_ptr<VulkanDescriptorPool> descriptorPool;
        std::unique_ptr<VulkanUniformBuffer> materialBuffer;
        // Descriptor management
        uint32_t nextSetOffset = 0;
    public:
        static const VulkanVertexInput vertex_3P_3C_3N_2U;
    };


    class VulkanMaterialInstance : public MaterialInstance {
    public:
        VulkanMaterialInstance(VulkanMaterial *material, VulkanDescriptorSet &&descriptorSet,
                               uint32_t uniformBufferOffset)
                : material(material), descriptorSet(descriptorSet), uniformBufferOffset(uniformBufferOffset) {}

        ~VulkanMaterialInstance() override = default; // TODO: Reuse descriptor sets

        inline const VulkanDescriptorSet &getDescriptorSet() const { return descriptorSet; }

        inline VkPipelineLayout getPipelineLayout() const { return material->pipeline->getPipelineLayout(); }

        inline VkPipeline getPipeline() const { return material->pipeline->getPipeline(); }

    private:
        VulkanMaterial *material;
        VulkanDescriptorSet descriptorSet;
        uint32_t uniformBufferOffset;
    };

}

