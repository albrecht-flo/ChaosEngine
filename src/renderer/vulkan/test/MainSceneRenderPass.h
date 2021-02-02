#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <array>

#include "VulkanRenderPassOld.h"
#include "src/renderer/vulkan/context/VulkanDevice.h"
#include "src/renderer/vulkan/context/VulkanSwapChain.h"
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

    void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) override;

    void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) override;

    void cmdEnd(VkCommandBuffer &cmdBuf) override;

    void recreate() override;

    void destroy() override;

    void destroySwapChainDependent() override;

    MaterialRef createMaterial(const TexturePhongMaterial &material);

private:
    void createRenderPass();

    void createBufferedDescriptorSetLayout();

    void createBufferedDescriptorPool();

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
    // Pipelines
    DescriptorSetLayout descriptorSetLayoutCameraBuf{};
    DescriptorSetLayout descriptorSetLayoutMaterials{};
    DescriptorSetLayout descriptorSetLayoutLights{};
    PipelineLayout graphicsPipelineLayout{};
    VulkanPipeline graphicsPipeline;

    // The objects for uniform buffer linking
    VkDescriptorPool descriptorPoolCamera{};
    std::vector<VkDescriptorSet> descriptorSetsCamera{};
    VkDescriptorPool descriptorPoolMaterials{};
    std::vector<VkDescriptorSet> descriptorSetsMaterials{};
    VkDescriptorPool descriptorPoolLights{};
    std::vector<VkDescriptorSet> descriptorSetsLights{};
};

