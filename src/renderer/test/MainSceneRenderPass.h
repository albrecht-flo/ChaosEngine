#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <array>

#include "../VulkanRenderPass.h"
#include "../general/VulkanDevice.h"
#include "../general/VulkanSwapChain.h"
#include "../image/VulkanImage.h"
#include "../pipeline/VulkanPipeline.h"
#include "../pipeline/VulkanDescriptor.h"
#include "../data/Mesh.h"
#include "../data/VulkanTexture.h"
#include "../data/RenderObject.h"

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

class MainSceneRenderPass : public VulkanRenderPass {
public:
    MainSceneRenderPass() = default;

    MainSceneRenderPass(VulkanDevice *device, VulkanMemory *vulkanMemory, VulkanSwapChain *swapChain);

    ~MainSceneRenderPass() = default;

    void updateUniformBuffer(uint32_t currentImage, Camera &camera, LightObject &worldLight);

    virtual void cmdBegin(VkCommandBuffer &cmdBuf, uint32_t currentImage, VkFramebuffer framebuffer) override;

    virtual void cmdRender(VkCommandBuffer &cmdBuf, RenderObject &robj) override;

    virtual void cmdEnd(VkCommandBuffer &cmdBuf) override;

    virtual void recreate() override;

    virtual void destroy() override;

    virtual void destroySwapChainDependent() override;

    MaterialRef createMaterial(const TexturePhongMaterial material);

private:
    void createRenderPass();

    void createBufferedDescriptorSetLayout();

    void createBufferedDescriptorPool();

    void createBufferedDescriptorSets();

    void createUniformBuffers();

    void createLightStructures();

private:
    std::unordered_map<std::string, VulkanTexture> m_textures;

private: // Internal buffer data
    UniformBufferContent<UniformBufferObject> m_uboContent;
    std::vector<UniformBufferContent<UniformMaterialObject>> m_materialUBContent;
    std::array<UniformBufferContent<UniformLightsObject>, 2> m_lightsUBContent;
    // The uniform buffers
    std::vector<VulkanUniformBuffer> m_uniformBuffers;
    std::vector<VulkanUniformBuffer> m_materialUniformBuffers;
    std::array<VulkanUniformBuffer, 2> m_lightUniformBuffers;
private:
    // Pipelines
    DescriptorSetLayout m_descriptorSetLayoutCameraBuf = {};
    DescriptorSetLayout m_descriptorSetLayoutMaterials = {};
    DescriptorSetLayout m_descriptorSetLayoutLights = {};
    PipelineLayout m_graphicsPipelineLayout;
    VulkanPipeline m_graphicsPipeline;

    // The objects for uniform buffer linking
    VkDescriptorPool m_descriptorPoolCamera = {};
    std::vector<VkDescriptorSet> m_descriptorSetsCamera;
    VkDescriptorPool m_descriptorPoolMaterials = {};
    std::vector<VkDescriptorSet> m_descriptorSetsMaterials;
    VkDescriptorPool m_descriptorPoolLights = {};
    std::vector<VkDescriptorSet> m_descriptorSetsLights;
};

