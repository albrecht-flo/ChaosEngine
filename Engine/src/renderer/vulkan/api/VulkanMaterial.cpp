#include "VulkanMaterial.h"

#include "Engine/src/core/utils/Logger.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSetLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPoolBuilder.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "VulkanRenderMesh.h"

using namespace Renderer;

static VulkanVertexInput convertToVulkanVertexLayout(const VertexLayout &layout) {
    auto builder = VertexAttributeBuilder(layout.binding, layout.stride, layout.inputRate);
    for (const auto &attribute: layout.attributes) {
        builder.addAttribute(attribute.location, attribute.format, (uint32_t) attribute.offset);
    }
    return builder.build();
}

// ------------------------------------ Class Members ------------------------------------------------------------------

VulkanMaterial::VulkanMaterial(GraphicsContext &pContext, const RendererAPI &renderer,
                               const MaterialCreateInfo &pInfo)
        : Material(pContext), info(pInfo) {
    auto &vulkanContext = dynamic_cast<VulkanContext &>(pContext);

    // Setup FixedFunction state indicators
    nonSolid = (pInfo.fixedFunction.polygonMode == Renderer::PolygonMode::Line);

    // Build layout for set-0
    if (info.set0) {
        auto builder = VulkanDescriptorSetLayoutBuilder(vulkanContext.getDevice());

        for (uint32_t i = 0; i < info.set0.value().size(); ++i) {
            auto binding = info.set0.value()[i];
            builder.addBinding(i, binding.type, binding.stage);
        }
        set0 = std::make_unique<VulkanDescriptorSetLayout>(builder.build());
    }

    // Build layout for set-1
    if (info.set1) {
        assert("For set 1 to be active set 0 is required" && set0);
        auto builder = VulkanDescriptorSetLayoutBuilder(vulkanContext.getDevice());

        for (uint32_t i = 0; i < info.set1.value().size(); ++i) {
            auto binding = info.set1.value()[i];
            builder.addBinding(i, binding.type, binding.stage);
            if (binding.type == ShaderBindingType::UniformBuffer) {
                assert("Only one Uniform buffer is supported at this point" && materialBufferSize == 0);
                for (const auto &value: *binding.layout) {
                    materialBufferSize += getSizeOfShaderValueType(value.type);
                }
            }
        }
        set1 = std::make_unique<VulkanDescriptorSetLayout>(builder.build());
    }

    // Build Pipeline layout
    auto pipelineLayoutBuilder = VulkanPipelineLayoutBuilder(vulkanContext.getDevice());
    if (set0) pipelineLayoutBuilder.addDescriptorSet(**set0);
    if (set1) pipelineLayoutBuilder.addDescriptorSet(**set1);
    if (info.pushConstant) {
        for (const auto &binding: info.pushConstant.value()) {
            pipelineLayoutBuilder.addPushConstant(getSizeOfShaderValueType(binding.type), binding.offset,
                                                  binding.stage);
        }
    }
    auto pipelineLayout = pipelineLayoutBuilder.build();

    const auto &renderPass = dynamic_cast<const VulkanRenderPass &>(renderer.getRenderPassForShaderStage(info.stage));

    // Build Pipeline
    pipeline = std::make_unique<VulkanPipeline>(
            VulkanPipelineBuilder(vulkanContext.getDevice(), renderPass,
                                  std::move(pipelineLayout),
                                  convertToVulkanVertexLayout(pInfo.vertexLayout),
                                  pInfo.name)
                    .setVertexShader(info.vertexShader)
                    .setFragmentShader(info.fragmentShader)
                    .setTopology(info.fixedFunction.topology)
                    .setPolygonMode(info.fixedFunction.polygonMode)
                    .setCullFace(info.fixedFunction.cullMode)
                    .setDepthTestEnabled(info.fixedFunction.depthTest)
                    .setDepthCompare(Renderer::CompareOp::Less)
                    .setAlphaBlendingEnabled(info.fixedFunction.alphaBlending)
                    .build());

    // Build descriptor pool
    auto descriptorPoolBuilder = VulkanDescriptorPoolBuilder(vulkanContext.getDevice());
    if (set0) {
        for (uint32_t i = 0; i < info.set0->size(); ++i) {
            descriptorPoolBuilder.addDescriptor(set0.value()->getBinding(i).descriptorType, info.set0ExpectedCount);
        }
    }
    if (set1) {
        assert("Set1 is defined but expected to be empty" && info.set1ExpectedCount > 0);
        for (uint32_t i = 0; i < info.set1->size(); ++i) {
            descriptorPoolBuilder.addDescriptor(set1.value()->getBinding(i).descriptorType, info.set1ExpectedCount);
        }
    }
    descriptorPoolBuilder.setMaxSets(info.set0ExpectedCount + info.set1ExpectedCount);
    descriptorPool = std::make_unique<VulkanDescriptorPool>(descriptorPoolBuilder.build());

    // Create material uniform buffer
    if (set1 && materialBufferSize > 0) {
        materialBuffer = std::make_unique<VulkanUniformBuffer>(vulkanContext.getMemory().createUniformBuffer(
                vulkanContext.getMemory().sizeWithUboPadding(materialBufferSize),
                info.set1ExpectedCount, false));
    }

}

std::shared_ptr<MaterialInstance>
VulkanMaterial::instantiate(std::shared_ptr<Material> &materialPtr, const void *materialData, uint32_t size,
                            const std::vector<const Texture *> &textures) {
    assert("Instantiating a destroyed material is impossible" &&
           (materialBufferSize == 0 || materialBuffer != nullptr));
    assert("Material uniform buffer needs to be filled completely" && size == materialBufferSize);
    auto &vulkanContext = dynamic_cast<VulkanContext &>(context);

    // Allocate/Ruse descriptor set
    uint32_t currentOffset = (freeDescSets.empty()) ? nextSetOffset : freeDescSets.back().first;
    VulkanDescriptorSet descriptorSet = (freeDescSets.empty()) ?
                                        descriptorPool->allocate(**set1) : freeDescSets.back().second;
    if (!freeDescSets.empty()) freeDescSets.pop_back();

    // Upload uniform data
    if (materialData != nullptr) {
        if (currentOffset >= materialBuffer->getSize())
            throw std::runtime_error("Uniform buffer is already full");

        // Copy the data to the uniform buffer
        vulkanContext.getMemory().copyDataToBuffer(materialBuffer->getBuffer(), materialData,
                                                   size, currentOffset);
    }

    // Update descriptor set-1 to the resources for this instance
    auto writer = descriptorSet.startWriting();
    auto texturesIt = textures.begin();
    for (uint32_t i = 0; i < info.set1.value().size(); ++i) {
        auto binding = info.set1.value()[i];
        switch (binding.type) {
            case ShaderBindingType::UniformBuffer:
                writer.writeBuffer(i, materialBuffer->getBuffer().vk(), currentOffset,
                                   materialBufferSize);
                break;
            case ShaderBindingType::TextureSampler:
                if (texturesIt == textures.end())
                    throw std::runtime_error("Missing textures.");
                const auto *tex = dynamic_cast<const VulkanTexture *>(*texturesIt);
                writer.writeImageSampler(i, tex->getSampler(), tex->getImageView(), tex->getImageLayout());
                ++texturesIt;
                break;
        }
    }
    writer.commit();
    auto instance = std::make_unique<VulkanMaterialInstance>(materialPtr, std::move(descriptorSet), currentOffset);

    // Set offset data for the uniform buffer accordingly
    auto paddedSize = vulkanContext.getMemory().sizeWithUboPadding(materialBufferSize);
    if (currentOffset == nextSetOffset)
        nextSetOffset += paddedSize;
    return instance;
}
