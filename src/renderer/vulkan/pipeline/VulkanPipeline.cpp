#include "VulkanPipeline.h"

#include <stdexcept>
#include <fstream>
#include <iostream>

/*	Creates shader modules
	Configure vertex input
	Configure primitive types
	Configure viewport
	Configure rasterizer
	Configure depth testing
	Configure blending
	Build pipeline
*/
VulkanPipeline VulkanPipeline::create(VulkanDevice &device,
                                      VkVertexInputBindingDescription bindingDescription,
                                      VkVertexInputAttributeDescription *attributeDesciption, uint32_t attributeCount,
                                      VkExtent2D swapChainExtent,
                                      PipelineLayout descriptorLayout,
                                      VkRenderPass renderPass,
                                      std::string shaderName,
                                      bool depthTestEnabled) {
    // Create shader objects
    auto vertShaderCode = readFile("shaders/" + shaderName + ".vert.spv");
    auto fragShaderCode = readFile("shaders/" + shaderName + ".frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // shader type
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // shader type
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Describes data format passed to the shader pipeline
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeCount;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDesciption;

    // Describes primitive type and reset  (GL_TRIANGLE)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE; // if true 0xffff and 0xffffffff will restart in index buffer (only STRIPS)

    // Define viewport
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Optional apply scissors to viewport
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Configure Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // -> discards fragments outside instead of clamping ! requires GPU feature
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // -> enables rasterizer output !!! otherwise disables framebuffer output
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // fill, line, point / Vertex draw mode
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // define culling
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // define vertex ordering for back and front
    rasterizer.depthBiasEnable = VK_FALSE; // can be useful for shadow maps

    // Configure Multisampling for fragment stage
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE; // disabled for the moment -> tocome
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Configure depth testing
    VkPipelineDepthStencilStateCreateInfo depthTesting = {};
    depthTesting.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthTesting.depthTestEnable = (depthTestEnabled) ? VK_TRUE : VK_FALSE;
    depthTesting.depthWriteEnable = (depthTestEnabled) ? VK_TRUE : VK_FALSE;
    depthTesting.depthCompareOp = VK_COMPARE_OP_LESS;
    depthTesting.depthBoundsTestEnable = VK_FALSE;
    depthTesting.minDepthBounds = 0.0f; // Optional
    depthTesting.maxDepthBounds = 1.0f; // Optional
    // Stencil
    depthTesting.stencilTestEnable = VK_FALSE;
    depthTesting.front = {};
    depthTesting.back = {};

    // Defines color attatchment blending -> used for alpha blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT; // color channels to be passed through, get ANDed with the color
    colorBlendAttachment.blendEnable = VK_FALSE; // disable mixing functions
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    // Manages color attatchment blending ops
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Create pipeline layout from descriptor set layouts
    std::vector<VkDescriptorSetLayout> setLayouts;
    for (auto &layout : descriptorLayout.layouts) {
        setLayouts.emplace_back(layout.vDescriptorSetLayout);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayout.layouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(descriptorLayout.pushConstants.size());
    pipelineLayoutInfo.pPushConstantRanges = descriptorLayout.pushConstants.data();

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(device.vk(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create pipeline layout!");
    }

    // Build pipeline with all configurations
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthTesting;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device.vk(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) !=
        VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create graphics pipeline!");
    }

    // No longer needed
    vkDestroyShaderModule(device.vk(), fragShaderModule, nullptr);
    vkDestroyShaderModule(device.vk(), vertShaderModule, nullptr);

    return VulkanPipeline(pipeline, pipelineLayout);
}


/* Create shader module form byte code */
VkShaderModule VulkanPipeline::createShaderModule(VulkanDevice &device, const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device.vk(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create shader module!");
    }

    return shaderModule;
}

/* Reads a file from 'filename' and returns it in bytes. */
std::vector<char> VulkanPipeline::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("VULKAN: failed to open file! " + filename);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void VulkanPipeline::destroy(VulkanDevice &device) {
    if (pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device.vk(), pipeline, nullptr);
    if (pipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device.vk(), pipelineLayout, nullptr);
}