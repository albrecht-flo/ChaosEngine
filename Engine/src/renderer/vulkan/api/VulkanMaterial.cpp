#include "VulkanMaterial.h"

#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSetLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineLayoutBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanPipelineBuilder.h"
#include "Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPoolBuilder.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanRenderPass.h"

#include <utility>

using namespace Renderer;

// TODO: fins a better place
static size_t sizeWithUboPadding(const VulkanContext &context, size_t originalSize) {
    // Source https://github.com/SaschaWillems/Vulkan/tree/master/examples/dynamicuniformbuffer
    // Calculate required alignment based on minimum device offset alignment
    size_t minUboAlignment = context.getDevice().getProperties().limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = originalSize;
    if (minUboAlignment > 0) {
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    return alignedSize;
}

const VulkanVertexInput VulkanMaterial::vertex_3P_3C_3N_2U =
        VertexAttributeBuilder(0, sizeof(Vertex), InputRate::Vertex)
                .addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos))
                .addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color))
                .addAttribute(2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal))
                .addAttribute(3, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)).build();

VulkanMaterial::VulkanMaterial(const GraphicsContext &pContext, const RendererAPI &renderer,
                               MaterialCreateInfo pInfo)
        : context(dynamic_cast<const VulkanContext &>(pContext)), info(std::move(pInfo)) {

    if (info.set0) {
        auto builder = VulkanDescriptorSetLayoutBuilder(context.getDevice());

        for (uint32_t i = 0; i < info.set0.value().size(); ++i) {
            auto binding = info.set0.value()[i];
            builder.addBinding(i, binding.type, binding.stage);
        }
        set0 = std::make_unique<VulkanDescriptorSetLayout>(builder.build());
    }

    if (info.set1) {
        assert("For set 1 to be active set 0 is required" && set0);
        auto builder = VulkanDescriptorSetLayoutBuilder(context.getDevice());

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
    auto pipelineLayoutBuilder = VulkanPipelineLayoutBuilder(context.getDevice());
    if (set0) pipelineLayoutBuilder.addDescriptorSet(**set0);
    if (set1) pipelineLayoutBuilder.addDescriptorSet(**set1);
    if (info.pushConstant) {
        for (const auto &binding : info.pushConstant.value()) {
            pipelineLayoutBuilder.addPushConstant(getSizeOfShaderValueType(binding.type), binding.offset,
                                                  binding.stage);
        }
    }
    auto pipelineLayout = pipelineLayoutBuilder.build();

    const auto &renderPass = dynamic_cast<const VulkanRenderPass &>(renderer.getRenderPassForShaderStage(info.stage));

    pipeline = std::make_unique<VulkanPipeline>(VulkanPipelineBuilder(context.getDevice(), renderPass,
                                                                      std::move(pipelineLayout), vertex_3P_3C_3N_2U,
                                                                      "")
                                                        .setVertexShader(info.vertexShader)
                                                        .setFragmentShader(info.fragmentShader)
                                                        .setTopology(info.fixedFunction.topology)
                                                        .setPolygonMode(info.fixedFunction.polygonMode)
                                                        .setCullFace(info.fixedFunction.cullMode)
                                                        .setDepthTestEnabled(info.fixedFunction.depthTest)
                                                                // TODO: depth write
                                                        .setDepthCompare(Renderer::CompareOp::Less)
                                                        .build());

    auto descriptorPoolBuilder = VulkanDescriptorPoolBuilder(context.getDevice());
    if (set0) {
        for (int i = 0; i < info.set0->size(); ++i) {
            descriptorPoolBuilder.addDescriptor(set0.value()->getBinding(i).descriptorType, info.set0ExpectedCount);
        }
    }
    if (set1) {
        assert("Set1 is defined but expected to be empty" && info.set1ExpectedCount > 0);
        for (int i = 0; i < info.set1->size(); ++i) {
            descriptorPoolBuilder.addDescriptor(set1.value()->getBinding(i).descriptorType, info.set1ExpectedCount);
        }
    }
    descriptorPoolBuilder.setMaxSets(info.set0ExpectedCount + info.set1ExpectedCount);
    descriptorPool = std::make_unique<VulkanDescriptorPool>(descriptorPoolBuilder.build());

    if (set1 && materialBufferSize > 0) {
        materialBuffer = std::make_unique<VulkanUniformBuffer>(context.getMemory().createUniformBuffer(
                sizeWithUboPadding(context, materialBufferSize),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                info.set1ExpectedCount, false));
    }

}

std::unique_ptr<MaterialInstance>
VulkanMaterial::instantiate(const void *materialData, uint32_t size, const std::vector<const Texture *> &textures) {
    {
        // TODO Defaults
        assert("Material uniform buffer needs to be filled completely" &&
               size == materialBufferSize);
        if(materialData != nullptr) {
            if (nextSetOffset >= materialBuffer->size)
                throw std::runtime_error("Uniform buffer is already full");

            // Copy that data to the uniform buffer
            context.getMemory().copyDataToBuffer(materialBuffer->buffer, materialBuffer->memory, materialData,
                                                 size, nextSetOffset);
        }

        VulkanDescriptorSet descriptorSet = descriptorPool->allocate(**set1);
        auto writer = descriptorSet.startWriting();
        auto texturesIt = textures.begin();
        for (uint32_t i = 0; i < info.set1.value().size(); ++i) {
            auto binding = info.set1.value()[i];
            switch (binding.type) {
                case ShaderBindingType::UniformBuffer:
                    writer.writeBuffer(i, materialBuffer->buffer, nextSetOffset,
                                       materialBufferSize); // TODO: This is broken
                    break;
                case ShaderBindingType::TextureSampler:
                    if (texturesIt == textures.end())
                        throw std::runtime_error("Missing textures.");
                    const auto *tex = dynamic_cast<const VulkanTexture *>(*texturesIt);
                    writer.writeImageSampler(i, tex->getSampler(), tex->getImageView().vk(), tex->getImageLayout());
                    ++texturesIt;
                    break;
            }
        }
        writer.commit();
        auto ret = std::make_unique<VulkanMaterialInstance>(this, std::move(descriptorSet), nextSetOffset);

        auto paddedSize = sizeWithUboPadding(context, materialBufferSize);
        nextSetOffset += paddedSize;
        return ret;
    };
}
