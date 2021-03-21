#pragma once

#include <Engine/src/renderer/data/Mesh.h>
#include <Engine/src/renderer/vulkan/context/VulkanContext.h>
#include <Engine/src/renderer/vulkan/pipeline/VulkanVertexInput.h>
#include <Engine/src/renderer/vulkan/pipeline/VulkanDescriptorSetLayout.h>
#include <Engine/src/renderer/vulkan/pipeline/VulkanDescriptorPool.h>
#include <Engine/src/renderer/vulkan/pipeline/VulkanPipeline.h>
#include <Engine/src/renderer/vulkan/image/VulkanTexture.h>
#include <Engine/src/core/RenderingSystem.h>

#include <utility>
#include "Engine/src/renderer/api/Material.h"
#include "Engine/src/renderer/api/RendererAPI.h"

namespace Renderer {
    class VulkanMaterialInstance;

    // TODO: Double buffering of Descriptor sets
    class VulkanMaterial : public Material {
        friend class VulkanMaterialInstance;

    private:
        VulkanMaterial(GraphicsContext &pContext, const RendererAPI &renderer, MaterialCreateInfo pInfo);

    public:
        ~VulkanMaterial() override = default;

        VulkanMaterial(const VulkanMaterial &o) = delete;

        VulkanMaterial &operator=(const VulkanMaterial &o) = delete;

        VulkanMaterial(VulkanMaterial &&o)
                : Material(o.context), info(o.info), set0(std::move(o.set0)), set1(std::move(o.set1)),
                  materialBufferSize(o.materialBufferSize), pipeline(std::move(o.pipeline)),
                  descriptorPool(std::move(o.descriptorPool)), materialBuffer(std::move(o.materialBuffer)),
                  nextSetOffset(o.nextSetOffset), freeDescSets(std::move(o.freeDescSets)) {}

        VulkanMaterial &operator=(VulkanMaterial &&o) = delete;

        static std::shared_ptr<VulkanMaterial>
        Create(GraphicsContext &pContext, const RendererAPI &renderer, MaterialCreateInfo pInfo) {
            return std::make_shared<VulkanMaterial>(VulkanMaterial(std::forward<GraphicsContext &>(pContext),
                                                                   std::forward<const RendererAPI &>(renderer),
                                                                   std::forward<MaterialCreateInfo>(pInfo)));
        }

        std::shared_ptr<MaterialInstance>
        instantiate(std::shared_ptr<Material> &materialPtr, const void *materialData, uint32_t size,
                    const std::vector<const Texture *> &textures) override;

        void recycleInstance(VulkanDescriptorSet &&descriptorSet, uint32_t uniformBufferOffset) {
            freeDescSets.emplace_back(uniformBufferOffset, std::move(descriptorSet));
        }

    private:
        MaterialCreateInfo info;
        std::optional<std::unique_ptr<VulkanDescriptorSetLayout>> set0 = std::nullopt;
        std::optional<std::unique_ptr<VulkanDescriptorSetLayout>> set1 = std::nullopt;
        uint32_t materialBufferSize = 0;
        std::unique_ptr<VulkanPipeline> pipeline;
        std::unique_ptr<VulkanDescriptorPool> descriptorPool;
        std::unique_ptr<VulkanUniformBuffer> materialBuffer;
        // Descriptor management
        uint32_t nextSetOffset = 0;
        std::vector<std::pair<uint32_t, VulkanDescriptorSet>> freeDescSets;
    public:
        static const VulkanVertexInput vertex_3P_3C_3N_2U;
    };


    /**
     * This class contains one instance of a descriptor consisting of pointer into the uniform buffer of the material
     * and texture samplers. <br>
     */
    class VulkanMaterialInstance : public MaterialInstance {
    private:
        /**
         * This class handles the buffered destruction of a material instance taking with it the material if there are
         * no other shared pointers to the material.
         */
        class VulkanMaterialInstanceBufferedDestroy : public BufferedGPUResource {
        public:
            VulkanMaterialInstanceBufferedDestroy(std::shared_ptr<Material> &&material,
                                                  VulkanDescriptorSet descriptorSet,
                                                  uint32_t uniformBufferOffset)
                    : material(std::move(material)), descriptorSet(descriptorSet),
                      uniformBufferOffset(uniformBufferOffset) {}

            ~VulkanMaterialInstanceBufferedDestroy() override = default;

            /// Notifies the parent material that the resources of this material instance can be recycled.
            void destroy() override {
                dynamic_cast<VulkanMaterial *>(material.get())->
                        recycleInstance(std::move(descriptorSet), uniformBufferOffset);
                material = nullptr;
            }

        private:
            std::shared_ptr<Material> material;
            VulkanDescriptorSet descriptorSet;
            uint32_t uniformBufferOffset;
        };

    public:
        VulkanMaterialInstance(std::shared_ptr<Material> material, VulkanDescriptorSet &&descriptorSet,
                               uint32_t uniformBufferOffset)
                : material(std::move(material)), descriptorSet(descriptorSet),
                  uniformBufferOffset(uniformBufferOffset) {}

        VulkanMaterialInstance(const VulkanMaterialInstance &o) = delete;

        VulkanMaterialInstance &operator=(const VulkanMaterialInstance &o) = delete;

        VulkanMaterialInstance(VulkanMaterialInstance &&o) noexcept
                : material(std::move(o.material)), descriptorSet(std::move(o.descriptorSet)),
                  uniformBufferOffset(o.uniformBufferOffset) {}

        VulkanMaterialInstance &operator=(VulkanMaterialInstance &&o) = delete;

        /**
         * Because a Material instance might currently be in use on the GPU for upto <i>maxFramesInFlight</i> frames
         * it needs to be destroyed using the buffered GPU resource destruction.
         * for this to be accomplished the destructor registers a destruction Object with the VulkanContext handing over
         * the ownership of the parent material.
         */
        ~VulkanMaterialInstance() override {
            auto &vulkanContext = dynamic_cast<VulkanContext &>(material->getContext());
            vulkanContext.destroyBuffered(
                    std::make_unique<VulkanMaterialInstanceBufferedDestroy>(
                            std::move(material), std::move(descriptorSet), uniformBufferOffset
                    ));
        }

        inline const VulkanDescriptorSet &getDescriptorSet() const { return descriptorSet; }

        inline VkPipelineLayout
        getPipelineLayout() const { return dynamic_cast<VulkanMaterial *>(material.get())->pipeline->getPipelineLayout(); }

        inline VkPipeline
        getPipeline() const { return dynamic_cast<VulkanMaterial *>(material.get())->pipeline->getPipeline(); }


    private:
        std::shared_ptr<Material> material;
        VulkanDescriptorSet descriptorSet;
        uint32_t uniformBufferOffset;
    };

}

