#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "VulkanDescriptorSetLayout.h"
#include "VulkanPipelineLayout.h"

/* TODO:
 *  // Later
 *  - Shader loading from disk should be managed in resource management.
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


    [[nodiscard]] inline VkPipeline getPipeline() const { return pipeline; }

    [[nodiscard]] inline VkPipelineLayout getPipelineLayout() const { return pipelineLayout.vk(); }

    inline VulkanPipelineLayout releasePipelineLayout() { return std::move(pipelineLayout); }

private:
    const VulkanDevice &device;
    VkPipeline pipeline;
    VulkanPipelineLayout pipelineLayout;
};