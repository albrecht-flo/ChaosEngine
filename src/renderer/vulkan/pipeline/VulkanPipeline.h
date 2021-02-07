#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "VulkanDescriptorSetLayout.h"

/* TODO:
 *  // Later
 *  - Shader loading from disk should be managed in ressource management.
 */
class VulkanPipelineBuilder;

class VulkanPipeline {
private:
    friend VulkanPipelineBuilder;

    VulkanPipeline(const VulkanDevice &device, VkPipeline pipeline, VulkanPipelineLayout &&pipelineLayout);

    void destroy();

public:
    ~VulkanPipeline();

    VulkanPipeline(const VulkanPipeline &o) = delete;

    VulkanPipeline &operator=(const VulkanPipeline &o) = delete;

    VulkanPipeline(VulkanPipeline &&o) noexcept;

    VulkanPipeline &operator=(VulkanPipeline &&o) noexcept;


    static VulkanPipeline Create(const VulkanDevice &device,
                                 VkVertexInputBindingDescription bindingDescription,
                                 const VkVertexInputAttributeDescription *attributeDesciption, uint32_t attributeCount,
                                 VkExtent2D swapChainExtent,
                                 VulkanPipelineLayout descriptorLayout,
                                 VkRenderPass renderPass,
                                 const std::string &shaderName,
                                 bool depthTestEnabled = true);

    [[nodiscard]] inline VkPipeline getPipeline() const { return pipeline; }

    [[nodiscard]] inline VkPipelineLayout getPipelineLayout() const { return pipelineLayout.vk(); }

    inline VulkanPipelineLayout releasePipelineLayout() { return std::move(pipelineLayout); }

private:
    const VulkanDevice &device;
    VkPipeline pipeline;
    VulkanPipelineLayout pipelineLayout;
};