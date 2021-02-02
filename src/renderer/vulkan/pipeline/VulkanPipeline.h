#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "VulkanDescriptor.h"

struct PipelineLayout {
    std::vector<DescriptorSetLayout> layouts;
    std::vector<VkPushConstantRange> pushConstants;
};

/* TODO:
 *  - Use Builder Pattern, because this is getting insane.
 *  - Use Dynamic state for viewport size
 *  // Later
 *  - Shader loading from disk should be managed in ressource management.
 */

class VulkanPipeline {
private:
    VulkanPipeline(const VulkanDevice &device, VkPipeline pipeline, VkPipelineLayout pipelineLayout);

    void destroy();

public:
    [[deprecated]] VulkanPipeline(const VulkanDevice &device)
            : device(device), pipeline(nullptr), pipelineLayout(nullptr) {}

    ~VulkanPipeline();


    VulkanPipeline(const VulkanPipeline &o) = delete;

    VulkanPipeline &operator=(const VulkanPipeline &o) = delete;

    VulkanPipeline(VulkanPipeline &&o) noexcept;

    VulkanPipeline &operator=(VulkanPipeline &&o) noexcept;


    static VulkanPipeline Create(const VulkanDevice &device,
                                 VkVertexInputBindingDescription bindingDescription,
                                 VkVertexInputAttributeDescription *attributeDesciption, uint32_t attributeCount,
                                 VkExtent2D swapChainExtent,
                                 PipelineLayout descriptorLayout,
                                 VkRenderPass renderPass,
                                 const std::string &shaderName,
                                 bool depthTestEnabled = true);

    [[nodiscard]] inline const VkPipeline getPipeline() const { return pipeline; }

    [[nodiscard]] inline const VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }


private:
    const VulkanDevice &device;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};