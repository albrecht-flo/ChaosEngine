#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <vector>
#include <string>

#include "../general/VulkanDevice.h"
#include "VulkanDescriptor.h"

struct PipelineLayout {
    std::vector<DescriptorSetLayout> layouts;
    std::vector<VkPushConstantRange> pushConstants;
};

class VulkanPipeline {
// Factory
public:
    static VulkanPipeline create(VulkanDevice &device,
                                 VkVertexInputBindingDescription bindingDescription,
                                 VkVertexInputAttributeDescription *attributeDesciption, uint32_t attributeCount,
                                 VkExtent2D swapChainExtent,
                                 PipelineLayout descriptorLayout,
                                 VkRenderPass renderPass,
                                 std::string shaderName,
                                 bool depthTestEnabled = true);

private:
    static VkShaderModule createShaderModule(VulkanDevice &device, const std::vector<char> &code);

    static std::vector<char> readFile(const std::string &filename);

// Container
public:
    VulkanPipeline() :
            pipeline(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE) {}

    ~VulkanPipeline() {}

    void destroy(VulkanDevice &device);

private:
    VulkanPipeline(VkPipeline pipeline, VkPipelineLayout pipelineLayout) :
            pipeline(pipeline), pipelineLayout(pipelineLayout) {}

public: // can not be const because of copy operator =
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};