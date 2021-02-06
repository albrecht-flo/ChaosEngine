#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <array>
#include <src/renderer/vulkan/pipeline/VulkanVertexInput.h>
#include <src/renderer/vulkan/pipeline/VulkanDescriptorSet.h>

#include "VulkanRenderPassOld.h"
#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "src/renderer/vulkan/context/VulkanSwapChain.h"
#include "src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "src/renderer/vulkan/image/VulkanImage.h"
#include "src/renderer/vulkan/pipeline/VulkanPipeline.h"
#include "src/renderer/vulkan/pipeline/VulkanDescriptor.h"
#include "src/renderer/data/Mesh.h"
#include "src/renderer/vulkan/image/VulkanTexture.h"
#include "src/renderer/data/RenderObject.h"

/* The uniform object past to the shaders.
	alignas(16) ensures proper alignment with the shaders
	*/
struct UniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    glm::vec4 worldLightPosition;
    glm::vec4 worldLightColor;
};

struct UniformLightsObject {
    glm::vec4 lightsCount;
    LightObject sources[3];
};

struct UniformMaterialObject {
    float shininess = 50;
};

class MainSceneRenderPass : public VulkanRenderPassOld {
public:
    MainSceneRenderPass(VulkanDevice &device, VulkanMemory &vulkanMemory, VulkanSwapChain &swapChain);

    ~MainSceneRenderPass() = default;

    void init() override;

    void updateUniformBuffer(uint32_t currentImage, Camera &camera, LightObject &worldLight);

    void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer, uint32_t viewportWidth,
                  uint32_t viewportHeight) override;

    void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) override;

    void cmdEnd(VkCommandBuffer &cmdBuf) override;

    void recreate() override;

    void destroy() override;

    void destroySwapChainDependent() override;

    MaterialRef createMaterial(const TexturePhongMaterial &material);

    [[nodiscard]] inline VkRenderPass vk() const { return renderPass->vk(); }

private:
    void createBufferedDescriptorSetLayout();

    void createBufferedDescriptorSets();

    void createUniformBuffers();

    void createLightStructures();

private:
    std::unordered_map<std::string, VulkanTexture> textures{};

private: // Internal buffer data
    UniformBufferContent<UniformBufferObject> uboContent{};
    std::vector<UniformBufferContent<UniformMaterialObject>> materialUBContent{};
    std::array<UniformBufferContent<UniformLightsObject>, 2> lightsUBContent{};
    // The uniform buffers
    std::vector<VulkanUniformBuffer> uniformBuffers{};
    std::vector<VulkanUniformBuffer> materialUniformBuffers{};
    std::array<VulkanUniformBuffer, 2> lightUniformBuffers{};
private:
    std::unique_ptr<VulkanRenderPass> renderPass;
    // Pipelines
    std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayoutCameraBuf;
    std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayoutMaterials;
    std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayoutLights;
    std::unique_ptr<VulkanPipeline> graphicsPipeline;
    VulkanVertexInput vertex_3P_3C_3N_2U;

    // The objects for uniform buffer linking
    std::unique_ptr<VulkanDescriptorPool> descriptorPoolCamera{};
    std::vector<VulkanDescriptorSet> descriptorSetsCamera{};
    std::unique_ptr<VulkanDescriptorPool> descriptorPoolMaterials{};
    std::vector<VulkanDescriptorSet> descriptorSetsMaterials{};
    std::unique_ptr<VulkanDescriptorPool> descriptorPoolLights{};
    std::vector<VulkanDescriptorSet> descriptorSetsLights{};
};

