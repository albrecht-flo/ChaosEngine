#include "VulkanPipelineBuilder.h"

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanRenderPass.h"
#include "VulkanPipeline.h"

#include <stdexcept>
#include <fstream>
#include <cassert>

/* Create shader module form byte code */
static VkShaderModule createShaderModule(const VulkanDevice &device, const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device.vk(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create shader module!");
    }

    return shaderModule;
}

/* Reads a file from 'filename' and returns it in bytes. */
static std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("[Vulkan] Failed to open file! " + filename);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return std::move(buffer);
}

// ------------------------------------ Class Members ------------------------------------------------------------------

VulkanPipeline VulkanPipelineBuilder::build() {
    assert(("The Layout was moved and no new one was provided.", layoutValid));
    assert(!vertexShaderName.empty());
    assert(!fragmentShaderName.empty());

    // Create shader objects -------------------------------------------------------------------------------------------
    // TODO: Load from asset manager
    auto vertShaderCode = readFile("shaders/" + vertexShaderName + ".vert.spv");
    auto fragShaderCode = readFile("shaders/" + fragmentShaderName + ".frag.spv");

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

    // Describes data format passed to the shader pipeline -------------------------------------------------------------
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInput.getBindingDescription().size());
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInput.getAttributeDescriptions().size());
    vertexInputInfo.pVertexBindingDescriptions = vertexInput.getBindingDescription().data();
    vertexInputInfo.pVertexAttributeDescriptions = vertexInput.getAttributeDescriptions().data();

    // Describes primitive type and reset  (GL_TRIANGLE) ---------------------------------------------------------------
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = getVkTopology(topology);
    // if true 0xffff and 0xffffffff will restart in index buffer (only STRIPS)
    inputAssembly.primitiveRestartEnable = primitiveRestart ? VK_TRUE : VK_FALSE;

    // Define dynamic state for viewport -------------------------------------------------------------------------------
    VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 1;
    dynamicState.pDynamicStates = dynamicStates;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(viewportExtent.width);
    viewport.height = static_cast<float>(viewportExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Apply scissors to viewport, need to be updated dynamically
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = viewportExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Configure Rasterizer --------------------------------------------------------------------------------------------
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // -> discards fragments outside instead of clamping ! requires GPU feature
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // -> enables rasterizer output !!! otherwise disables framebuffer output
    rasterizer.polygonMode = getVkPolygonMode(polygonMode); // fill, line, point / Vertex draw mode
    rasterizer.lineWidth = 1.0f; // Must be set dynamically
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // define culling
    rasterizer.frontFace = getVkCullFace(cullFace); // define vertex ordering for back and front
    rasterizer.depthBiasEnable = VK_FALSE; // can be useful for shadow maps

    // Configure Multisampling for fragment stage ----------------------------------------------------------------------
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE; // disabled for the moment -> tocome
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Configure depth testing -----------------------------------------------------------------------------------------
    VkPipelineDepthStencilStateCreateInfo depthTesting = {};
    depthTesting.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthTesting.depthTestEnable = (depthTestEnabled) ? VK_TRUE : VK_FALSE;
    depthTesting.depthWriteEnable = (depthTestEnabled) ? VK_TRUE : VK_FALSE;
    depthTesting.depthCompareOp = getVkCompareOp(depthCompare);
    depthTesting.depthBoundsTestEnable = VK_FALSE;
    depthTesting.minDepthBounds = 0.0f; // Optional
    depthTesting.maxDepthBounds = 1.0f; // Optional
    // Stencil
    depthTesting.stencilTestEnable = VK_FALSE;
    depthTesting.front = {};
    depthTesting.back = {};

    // Defines color attatchment blending -> used for alpha blending ---------------------------------------------------
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

    // Build pipeline with all configurations --------------------------------------------------------------------------
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState; // To be set at runtime via vkCmdSetViewport
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthTesting;
//    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = layout.vk();
    pipelineInfo.renderPass = renderPass.vk();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline pipeline{};
    if (vkCreateGraphicsPipelines(device.vk(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) !=
        VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create graphics pipeline!");
    }

    // No longer needed
    vkDestroyShaderModule(device.vk(), fragShaderModule, nullptr);
    vkDestroyShaderModule(device.vk(), vertShaderModule, nullptr);

    layoutValid = false; // It gets moved so we need a new one for the next build

    return VulkanPipeline{device, pipeline, std::move(layout)};
}

