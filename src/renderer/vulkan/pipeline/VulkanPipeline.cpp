#include "VulkanPipeline.h"

#include "src/renderer/vulkan/context/VulkanDevice.h"

#include <stdexcept>
#include <fstream>

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

VulkanPipeline::VulkanPipeline(const VulkanDevice &device, VkPipeline pipeline, VulkanPipelineLayout &&pipelineLayout)
        : device(device), pipeline(pipeline), pipelineLayout(std::move(pipelineLayout)) {}

VulkanPipeline::~VulkanPipeline() {
    destroy();
}

VulkanPipeline::VulkanPipeline(VulkanPipeline &&o) noexcept
        : device(o.device), pipeline(std::exchange(o.pipeline, nullptr)),
          pipelineLayout(std::move(o.pipelineLayout)) {}

VulkanPipeline &VulkanPipeline::operator=(VulkanPipeline &&o) noexcept {
    if (this == &o)
        return *this;
    destroy();
    pipeline = std::exchange(o.pipeline, nullptr);
    pipelineLayout = std::move(o.pipelineLayout);
    return *this;
}

void VulkanPipeline::destroy() {
    if (pipeline != nullptr)
        vkDestroyPipeline(device.vk(), pipeline, nullptr);
}
