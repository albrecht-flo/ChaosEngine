#include "VulkanPipeline.h"

#include "Engine/src/renderer/vulkan/context/VulkanDevice.h"
#include "VulkanPipelineLayout.h"

#include <stdexcept>
#include <fstream>

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
